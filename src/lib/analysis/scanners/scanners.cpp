#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <queue>
#include <set>

#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "connectionInitiator.hpp"
#include "modules.hpp"
#include "diskMultimap.hpp"
#include "MiscHelpers.hpp"

#include "scanners.hpp"

using namespace std;
using namespace tr1;

#define SCANNERS_TABLE_SCHEMA "\"sourceIP\" uint32 NOT NULL, \
								 \"destinationIP\" uint32 NOT NULL, \
								 \"destinationPort\" uint16 NOT NULL, \
								 \"startTime\" uint32 NOT NULL, \
								 \"endTime\" uint32 NOT NULL, \
								 \"scanType\" TEXT NOT NULL, \
								 \"successfulCount\" uint32 NOT NULL, \
								 \"failedCount\" uint32 NOT NULL, \
								 \"numBytes\" uint32 NOT NULL, \
								 \"numPackets\" uint32 NOT NULL, \
								 \"asNumber\" uint16 NOT NULL, \
								 \"countryNumber\" SMALLINT NOT NULL, \
									PRIMARY KEY (\"sourceIP\",
												 \"destinationIP\", \
												 \"destinationPort\")"


TwoWayHostPair hostPair;
size_t tcpTimeout;
size_t minIPs,minPorts;
uint16_t asn;
/// <Stores connection streams till their outcome is determined
map <TwoWayHostPair, States> connectionStates;	
map <TwoWayHostPair, States>::iterator connectionStatesItr;
map <TwoWayHostPair, States>::iterator tempItr;
/// <Stores connection stats for each sourceIP
map <uint32_t, ConnectionStats> scannerIPs;	
map <uint32_t, ConnectionStats>::iterator scannerIPsItr; 
/// <Stores reference to connection streams to check for TCP timeout
queue <TwoWayHostPair> timeoutQ;	
/// <Stores ports to be monitored, parsed from monitoredServices.conf
set <uint16_t> monitoredPorts; 
set <uint16_t>:: iterator monitoredPortsEnd;
vector <pair <uint32_t, uint32_t> > *localNetworks;
/// Berkeley DB disk cache for destination IPs and Ports
DiskMultimap <uint32_t, Destination> destinationCache;
Destination destination;
int8_t initiator = TEMPORARILY_UNKNOWN_INITIATOR;


/*
 * TCP initiators. See header file for details. It should not be necessary to
 * call aggregate() on this class for any stage-2 modules because the
 * commChannels module aggregates all TCP flows and is in stage 1.
 */
ConnectionInitiators *connectionInitiators;

/* IP ASN and country information. See header file for details. */
IPInformation *ipInformation;

/*Map that obtains roles in shared state from parallel modules*/
std::tr1::unordered_multimap <uint32_t, Role> *roles;
std::tr1::unordered_multimap <uint32_t, Role>::iterator rolesItr;

///\Function to check if flow belongs to list of monitored ports

///\Function to check if destination port is a monitored port
inline bool isMonitoredPort(uint16_t destinationPort){
	if(monitoredPorts.find(destinationPort) != monitoredPortsEnd){
		return true;
	}
	else{
		return false;
	}
}

/// \brief Function to parse monitored TCP services from file
inline bool getMonitoredServices(string file_name){
	size_t firstSpace, startPosition, endPosition;
	string line;
	ifstream file(file_name);
	if(!file.is_open()){
		cerr<<"getMonitoredServices : "<<strerror(errno)<<endl;
		return false;
	}
	else{
		while(getline(file,line)){
			if( line.size() && line.at(0) != '#' && 
					line.at(0) != '\n' && line.at(0) != ' ')
			{
				firstSpace = line.find(' ');
				startPosition = line.find_first_not_of(' ',firstSpace);
				endPosition = line.find_first_of('/',startPosition);
				monitoredPorts.insert(boost::lexical_cast <uint16_t> (
									line.substr(startPosition,
												endPosition-startPosition)));
			}
		}
	}
	file.close();
	return true;
}

extern "C" {
	/*
	 * This function is called once, before the module is given any neoflow
	 * records. Returns true upon success, or false upon failure. The PGconn and
	 * date (in YYYY-MM-DD format) variables should be used in case it's necessary
	 * to get something from the database. The conf variable contains any
	 * configuration options from the module's configuration file, which always
	 * takes the module's name and a ".conf" suffix. Options are stored as
	 * std::string types and are indexed by their names in the configuration file
	 * (conf.options["optionName"]). The rest of the variables are pointers to
	 * state maintained by the main executable, and should be assigned to the
	 * global pointers to the state.
	 */
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		localNetworks = sharedState.localNetworks;
		connectionInitiators = sharedState.connectionInitiators;
		ipInformation = sharedState.ipInformation;
		roles = sharedState.roles;
		if (moduleState.conf.get(tcpTimeout,
								 "tcp-timeout",
								 "analysis_scanners")
				!= configuration::OK)
		{
			cerr << "analysis_scanners: missing or invalid tcp-timeout"
				 << endl;
			return false;
		}

		size_t hard_limit;
		if (moduleState.conf.get(hard_limit,
								 "hard-limit",
								 "analysis_scanners")
				!= configuration::OK)
		{
			cerr << "analysis_scanners: missing or invalid hard-limit"
				 << endl;
			return false;
		}

		string services_file;
		if (moduleState.conf.get(services_file,
								 "services-file",
								 "analysis_scanners")
				!= configuration::OK)
		{
			cerr << "analysis_scanners: missing or invalid services-file"
				 << endl;
			return false;
		}

		if (!destinationCache.initialize("/tmp/scanners.db", hard_limit)) {
			cerr<<destinationCache.error()<<endl;
			return 1;
		}
		if (!getMonitoredServices(services_file))
		{
			return false;
		}
		monitoredPortsEnd = monitoredPorts.end();
		return true;
	}


	/*
	 * This function is called for every neoflow record. The hour variable is
	 * the hour of the day (0 through 23) the record belongs to.
	 */
	void aggregate(const FlowStats *flowStats, size_t hour) {
		if(isTCP(flowStats) && isMonitoredFlow(flowStats)){

			//getting bidirectional flows to accumulate flags 
			hostPair = makeTwoWayHostPair(flowStats,*localNetworks);
			connectionStatesItr = connectionStates.find(hostPair);

			//checking if connection stream is already open
			if(connectionStatesItr == connectionStates.end()){
				//adding new connection stream to map and queue
				connectionStatesItr = connectionStates.insert(make_pair(hostPair, States())).first;
				timeoutQ.push(hostPair);
			}

			//update Flags seen for connection stream
			connectionStatesItr->second.tcpSYNs += flowStats->tcpSYNs();
			connectionStatesItr->second.tcpACKs += flowStats->tcpACKs();
			connectionStatesItr->second.tcpFINs += flowStats->tcpFINs();
			connectionStatesItr->second.tcpRSTs += flowStats->tcpRSTs();
			connectionStatesItr->second.tcpURGs += flowStats->tcpURGs();
			connectionStatesItr->second.tcpPUSHs += flowStats->tcpPUSHs();

			//updating connection start and end time
			compareLessThan(connectionStatesItr->second.startTime, flowStats->startTime().seconds());
			compareGreaterThan(connectionStatesItr->second.endTime, flowStats->endTime().seconds());

			//updating flows seen
			++connectionStatesItr->second.numFlows;

			//updating packets seen
			connectionStatesItr->second.numPackets += flowStats->numPackets();

			//updating number of bytes
			connectionStatesItr->second.numBytes += flowStats->numBytes();

			//set the SYN Time for connection stream
			if(flowStats->firstSYNTime().seconds()){
				connectionStatesItr->second.firstSYNTime = flowStats->firstSYNTime().seconds();
			}

			//set the initiator for the connection stream
			if(connectionStatesItr->second.initiator != INTERNAL_INITIATOR &&
				 connectionStatesItr->second.initiator != EXTERNAL_INITIATOR){
				 connectionStatesItr->second.initiator = connectionInitiators->getInitiator(hostPair,
																																				flowStats->sourceIP());
				if(connectionStatesItr->second.initiator == TEMPORARILY_UNKNOWN_INITIATOR){
					connectionInitiators->aggregate(hostPair, flowStats);
					connectionStatesItr->second.initiator = connectionInitiators->getInitiator(hostPair,
																																				 flowStats->sourceIP());
				}
				
				if(connectionStatesItr->second.initiator == INTERNAL_INITIATOR ||
					 connectionStatesItr->second.initiator == EXTERNAL_INITIATOR){
					//if initiator was obtained from connection initiators, 3-way handshake was seen
					connectionStatesItr->second.handshake = true;
				}
				else{					
					//setting the initiator based on SYN flag when no handshake was seen
					if(flowStats->tcpSYNs() && flowStats->firstSYNTime().seconds()){
						if(isInternal(flowStats->sourceIP(), *localNetworks)){
							connectionStatesItr->second.initiator = INTERNAL_INITIATOR;
						}
						else{
							connectionStatesItr->second.initiator = EXTERNAL_INITIATOR;
						}
					}
				}
			}
			
			//when initiator is known ..
			if(connectionStatesItr->second.initiator == INTERNAL_INITIATOR ||
				 connectionStatesItr->second.initiator == EXTERNAL_INITIATOR){
				//successful connection - SYN, ACK and FIN/RST with the handshake variable set
				if(connectionStatesItr->second.numFlows > 1 &&
					 ((connectionStatesItr->second.tcpSYNs && connectionStatesItr->second.tcpACKs && 
						connectionStatesItr->second.tcpFINs && connectionStatesItr->second.handshake) || 
						(connectionStatesItr->second.tcpSYNs && connectionStatesItr->second.tcpACKs &&
						 connectionStatesItr->second.tcpRSTs && connectionStatesItr->second.handshake)) ){
			if(connectionStatesItr->second.initiator == INTERNAL_INITIATOR){
						scannerIPsItr = scannerIPs.find(connectionStatesItr->first.internalIP);
						if(scannerIPsItr == scannerIPs.end()){
							scannerIPsItr = scannerIPs.insert(make_pair(connectionStatesItr->first.internalIP,
																																			ConnectionStats())).first;
						}
					}
			if(connectionStatesItr->second.initiator == EXTERNAL_INITIATOR){
						scannerIPsItr = scannerIPs.find(connectionStatesItr->first.externalIP);
						if(scannerIPsItr == scannerIPs.end()){
							scannerIPsItr = scannerIPs.insert(make_pair(connectionStatesItr->first.externalIP,
																																			ConnectionStats())).first;
						}
					}
					++scannerIPsItr->second.successful;
					
					//updating start and end time
					//compareLessThan(scannerIPsItr->second.startTime, connectionStatesItr->second.startTime);
					//compareGreaterThan(scannerIPsItr->second.endTime, connectionStatesItr->second.endTime);

					//updating number of packets
					//scannerIPsItr->second.numPackets += connectionStatesItr->second.numPackets;

					//updating number of bytes
					//scannerIPsItr->second.numBytes +=connectionStatesItr->second.numBytes;
					
					connectionStates.erase(connectionStatesItr);
				}

				//end of queue is checked for half open tcp connections that have been open for over 5 minutes
				while(!timeoutQ.empty()){
					if(connectionStates.find(timeoutQ.front()) == connectionStates.end()){
						timeoutQ.pop();
					}
					else{
						tempItr = connectionStates.find(timeoutQ.front());
						if(flowStats->endTime().seconds() - tempItr->second.firstSYNTime > tcpTimeout){
							//failed connection - SYN followed by NO ACK or SYN followed by RST
							if(tempItr->second.tcpSYNs && !tempItr->second.tcpACKs){
								if(tempItr->second.initiator == INTERNAL_INITIATOR && isMonitoredPort(tempItr->first.externalPort) ){
									scannerIPsItr = scannerIPs.find(tempItr->first.internalIP);
									if(scannerIPsItr == scannerIPs.end()){
										scannerIPsItr = scannerIPs.insert(make_pair(tempItr->first.internalIP,
																																			ConnectionStats())).first;
									}
									//updating destination ips and ports
									destination = makeDestinationPair(tempItr->first.externalIP, 
																											tempItr->first.externalPort);
									destinationCache.insert(make_pair(tempItr->first.internalIP,destination));

									++scannerIPsItr->second.failed;
									++scannerIPsItr->second.failedSYNs;
	 
									//updating start and end time
									compareLessThan(scannerIPsItr->second.startTime, tempItr->second.startTime);
									compareGreaterThan(scannerIPsItr->second.endTime, tempItr->second.endTime);

									//updating number of packets
									scannerIPsItr->second.numPackets += tempItr->second.numPackets;

									//updating number of bytes
									scannerIPsItr->second.numBytes += tempItr->second.numBytes;
								}
								if(tempItr->second.initiator == EXTERNAL_INITIATOR && isMonitoredPort(tempItr->first.internalPort)){
									scannerIPsItr = scannerIPs.find(tempItr->first.externalIP);
									if(scannerIPsItr == scannerIPs.end()){
										scannerIPsItr = scannerIPs.insert(make_pair(tempItr->first.externalIP,
																																			ConnectionStats())).first;
									}
									//updating destination ips and ports
									destination = makeDestinationPair(tempItr->first.internalIP, 
																												 tempItr->first.internalPort);
									destinationCache.insert(make_pair(tempItr->first.externalIP,destination));

									++scannerIPsItr->second.failed;
									++scannerIPsItr->second.failedSYNs;
	 
									//updating start and end time
									compareLessThan(scannerIPsItr->second.startTime, tempItr->second.startTime);
									compareGreaterThan(scannerIPsItr->second.endTime, tempItr->second.endTime);

									//updating number of packets
									scannerIPsItr->second.numPackets += tempItr->second.numPackets;

									//updating number of bytes
									scannerIPsItr->second.numBytes += tempItr->second.numBytes;
								}
								connectionStates.erase(tempItr);
							}
							timeoutQ.pop();					
						}
						else{
							break;
						}	
					}
				}
			}
			//if initiator of connection stream is unknown..
			else{
				//failed connection - FIN,ACK,PUSH and URG set (XMAS Tree scan)					 
				if(connectionStatesItr->second.numFlows == 1 &&
					 !(connectionStatesItr->second.tcpSYNs) && !(connectionStatesItr->second.tcpACKs) && 
					 connectionStatesItr->second.tcpFINs && connectionStatesItr->second.tcpURGs &&
																									connectionStatesItr->second.tcpPUSHs){
					//making sure it did not belong to a previous connection
					scannerIPsItr = scannerIPs.find(flowStats->destinationIP());
					if(scannerIPsItr == scannerIPs.end() && isMonitoredPort(flowStats->destinationPort())){
						scannerIPsItr = scannerIPs.find(flowStats->sourceIP());
							if(scannerIPsItr == scannerIPs.end()){
								scannerIPsItr = scannerIPs.insert(make_pair(flowStats->sourceIP(),
																																	 ConnectionStats())).first;
							}
							++scannerIPsItr->second.failed;
							++scannerIPsItr->second.failedXMASs;
			 
							//updating start and end time
							compareLessThan(scannerIPsItr->second.startTime, connectionStatesItr->second.startTime);
							compareGreaterThan(scannerIPsItr->second.endTime, connectionStatesItr->second.endTime);

							//updating number of packets
							scannerIPsItr->second.numPackets += connectionStatesItr->second.numPackets;
			 
							//updating number of bytes
							scannerIPsItr->second.numBytes += flowStats->numBytes();

							//updating destination ips and ports
							destination = makeDestinationPair(flowStats->destinationIP(), flowStats->destinationPort());
							destinationCache.insert(make_pair(flowStats->sourceIP(),destination));
					}
					connectionStates.erase(connectionStatesItr);
				}
				else{
					//failed connection - FIN without preceeding SYN or ACK					 
					if(connectionStatesItr->second.numFlows == 1 &&
						!(connectionStatesItr->second.tcpSYNs) && !(connectionStatesItr->second.tcpACKs) && 
																										connectionStatesItr->second.tcpFINs){
						//making sure FIN did not belong to a previous connection
						scannerIPsItr = scannerIPs.find(flowStats->destinationIP());
						if(scannerIPsItr == scannerIPs.end() && isMonitoredPort(flowStats->destinationPort())){ 
							scannerIPsItr = scannerIPs.find(flowStats->sourceIP());
							if(scannerIPsItr == scannerIPs.end()){
								scannerIPsItr = scannerIPs.insert(make_pair(flowStats->sourceIP(),
																																	 ConnectionStats())).first;
							}
							++scannerIPsItr->second.failed;
							++scannerIPsItr->second.failedFINs;
			
							//updating start and end time
							compareLessThan(scannerIPsItr->second.startTime, connectionStatesItr->second.startTime);
							compareGreaterThan(scannerIPsItr->second.endTime, connectionStatesItr->second.endTime);

							//updating number of bytes
							scannerIPsItr->second.numBytes += flowStats->numBytes();

							//updating number of packets
							scannerIPsItr->second.numPackets += connectionStatesItr->second.numPackets;
			
							//updating destination ips and ports
							destination = makeDestinationPair(flowStats->destinationIP(), flowStats->destinationPort());
							destinationCache.insert(make_pair(flowStats->sourceIP(),destination));
						}
						connectionStates.erase(connectionStatesItr);
					}
					else{
						//failed connection - ACK without preceeding SYN
						/* ACK scans require checks to ensure ACKs are not keep alive packets. This 
						 * scenario occurs when the TCP connection was initiated on a prior day and only 
						 * keep alives are seen in the current days flows. As opposed to other scans in 
						 * ACK scan we wait for further flows to see if the ACK scan elicted a RST in 
						 * response to categorize it as ACK scan
						 */					
						if(connectionStatesItr->second.numFlows > 1 &&
							 !(connectionStatesItr->second.tcpSYNs) && connectionStatesItr->second.tcpACKs &&
							 !(connectionStatesItr->second.tcpFINs) && flowStats->tcpRSTs()){
							//making sure ACK did not belong to a previous connection
							scannerIPsItr = scannerIPs.find(flowStats->sourceIP());
							if(scannerIPsItr == scannerIPs.end() && isMonitoredPort(flowStats->sourcePort())){
								 scannerIPsItr = scannerIPs.find(flowStats->destinationIP());
								 if(scannerIPsItr == scannerIPs.end()){
									 scannerIPsItr = scannerIPs.insert(make_pair(flowStats->destinationIP(),
																																	ConnectionStats())).first;
									}
									
									++scannerIPsItr->second.failed;
									++scannerIPsItr->second.failedACKs;
				
									//updating start and end time
									compareLessThan(scannerIPsItr->second.startTime, connectionStatesItr->second.startTime);
									compareGreaterThan(scannerIPsItr->second.endTime, connectionStatesItr->second.endTime);
			
									//updating number of bytes
									scannerIPsItr->second.numBytes += flowStats->numBytes();

									//updating number of packets
									scannerIPsItr->second.numPackets += connectionStatesItr->second.numPackets;
			 
									//updating destination ips and ports
									destination = makeDestinationPair(flowStats->sourceIP(), flowStats->sourcePort());
									destinationCache.insert(make_pair(flowStats->destinationIP(),destination));
							}
							connectionStates.erase(connectionStatesItr);
						}
						else{
							/* Initiator is unknown and connection was not initiated by a FIN or ACK
							 * indicating dangling RSTs/ACKS from previous connections 
							 */
							if(!(connectionStatesItr->second.tcpSYNs) && !(connectionStatesItr->second.tcpACKs) &&
								 !(connectionStatesItr->second.tcpFINs) && connectionStatesItr->second.tcpRSTs ){
								connectionStates.erase(connectionStatesItr);
							}
						}
					}
				}
			}
		}
	}
			
	/*
	 * This function is called once, after aggregate() has been called for each
	 * neoflow record. Typical things to do here are to prepare the appropriate
	 * SQL table using preparePGTable() commit any information gathered using
	 * the PGBulkInserter class, and clear any state kept by the module. Returns
	 * the number of SQL rows inserted upon success, or -1 upon failure.
	 */

	int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
		connectionStates.clear();
		
		set <uint32_t> p2pNodes;
		for(rolesItr = roles->begin(); rolesItr != roles->end(); ++rolesItr){
			if(rolesItr->second._role == MULTIMEDIA_P2P_NODE ||
				 rolesItr->second._role == UNCLASSIFIED_P2P_NODE ||
				 rolesItr->second._role == ENCRYPTED_P2P_NODE){
				p2pNodes.insert(rolesItr->first);
			}
		}

		if( !(p2pNodes.size())){
			cout<<"No P2P Traffic"<<endl;
		}

		set <uint32_t>:: iterator p2pNodesEnd(p2pNodes.end());

		const uint32_t *sourceIP = NULL;
		uint32_t prevSourceIP = NULL;
		Destination *destinationPair = NULL;

		Clock clock("Inserted", "rows");
		PGBulkInserter pgBulkInserter(pg_conn.connection(), SCANNERS_SCHEMA_NAME,
																	 date, flushSize, "%ud, %ud, %ud, %ud, %ud, " \
																	 "%s, %ud, %ud, %ud, %ud, %ud, %d");
		if (!preparePGTable(pg_conn.connection(), SCANNERS_SCHEMA_NAME, date,
												 SCANNERS_TABLE_SCHEMA)) {
				return -1;
		}

		if(!destinationCache.close()){
			cerr<<destinationCache.error()<<endl;
		}
		
		map <uint32_t, ConnectionStats>::iterator scannerIPsEnd(scannerIPs.end());
		//determining scan type
		for(scannerIPsItr = scannerIPs.begin(); scannerIPsItr != scannerIPs.end(); 
																															 ++scannerIPsItr){
			if(scannerIPsItr->second.failedSYNs){
			 scannerIPsItr->second.scanType += "SYN_SCAN";
			}
			if(scannerIPsItr->second.failedACKs){
				if(!scannerIPsItr->second.scanType.empty()){
					scannerIPsItr->second.scanType += ", ";
				}
				scannerIPsItr->second.scanType += "ACK_SCAN";
			}
			if(scannerIPsItr->second.failedFINs){
				if(!scannerIPsItr->second.scanType.empty()){
					scannerIPsItr->second.scanType += ", ";
				}
				scannerIPsItr->second.scanType += "FIN_SCAN";
			}
			if(scannerIPsItr->second.failedXMASs){
				if(!scannerIPsItr->second.scanType.empty()){
					scannerIPsItr->second.scanType += ", ";
				}
				scannerIPsItr->second.scanType += "XMAS_SCAN";
			}
		} 
			 
		if(!destinationCache.openForReading()){
			cerr<<destinationCache.error()<<endl;
			return -1;
		}

		set <Destination> seenDestinations;
		clock.start();
		cout << "commit(): Writing to pg_conn.connection()" << endl;
		while(destinationCache.read(sourceIP, destinationPair)){
			//Look for first non Duplicate record
			if(prevSourceIP != *sourceIP){
				seenDestinations.clear();
				scannerIPsItr = scannerIPs.find(*sourceIP);
			}
			prevSourceIP = *sourceIP;
			if(scannerIPsItr != scannerIPsEnd && p2pNodes.find(*sourceIP) == p2pNodesEnd &&
					(scannerIPsItr->second.successful + scannerIPsItr->second.failed > 10) &&
					(seqHypothesis(scannerIPsItr->second.successful, scannerIPsItr->second.failed) > 0.99) ){
				//Aggregate destination IPs and ports for consecutive duplicate records
				destination = makeDestinationPair(destinationPair->destinationIP, destinationPair->destinationPort);
				if (seenDestinations.find(destination) 
						== seenDestinations.end())
				{
					if (!pgBulkInserter.insert(NULL, scannerIPsItr -> first,
								destinationPair -> destinationIP,
								destinationPair -> destinationPort,
								scannerIPsItr -> second.startTime,
								scannerIPsItr -> second.endTime,
								scannerIPsItr -> second.scanType.c_str(),
								scannerIPsItr -> second.failed,
								scannerIPsItr -> second.successful,
								scannerIPsItr -> second.numBytes,
								scannerIPsItr -> second.numPackets,
								ipInformation -> getASN(
									destinationPair->destinationIP),
								ipInformation -> getCountry(
									destinationPair->destinationIP)))
					{
						return -1;
					}
					clock.incrementOperations();
					seenDestinations.insert(destination);
				}
			}
		}
		 
		cout << "commit(): Flushing PgSQL cache to pg_conn.connection()" << endl;
		if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
			return -1;
		}
		clock.stop();
		if(!destinationCache.close()){
			cerr<<destinationCache.error()<<endl;		 
		}
		scannerIPs.clear();
		return clock.operations();
	}
}
