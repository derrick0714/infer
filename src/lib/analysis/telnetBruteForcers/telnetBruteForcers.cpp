
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <boost/lexical_cast.hpp>
#include <set>

#include "bruteForcers.hpp"
#include "configuration.h"
#include "postgreSQL.h"
#include "clock.hpp"
#include "MiscHelpers.hpp"

using namespace std;

///Map that obtains roles in shared state from parallel modules
std::tr1::unordered_multimap <uint32_t, Role> *roles;
std::tr1::unordered_multimap <uint32_t, Role>::iterator rolesItr;

vector <pair <uint32_t, uint32_t> > *localNetworks;
///TCP initiators object of template type TwoWayHostPair. It detects the Initiator 
///of a TCP connection based on the three way Handshake.
ConnectionInitiators *connectionInitiators;

///Helps get the ASN and Country for IP's
IPInformation *ipInformation;

///Object of type TwoWayHostPair to get bidirectional flows
TwoWayHostPair hostPair;

///Object of type bruteHostPair used as key in the map
BruteHostPair brutePair;

///Map that stores internal and external bruteIP's as Key and object of type BruteForcers as value
map <BruteHostPair, BruteForcers> bruteIPs;
map <BruteHostPair, BruteForcers>::iterator bruteItr;

///statistical vectors
vector <uint32_t> numInits;      ///< Stores Number of handshakes of Brute IP's 
vector <TimeStamp> avgDuration;   ///< Stores minimum average duration of all sessions for Brute IP's
vector <double> bytesInitsRatio; ///<Stores correlation between data transferred and number of Initialization

///Statistical vaiables 
uint32_t numInitsMedian;  ///< Stores Median of Number of TCP Handshakes
TimeStamp avgDurationMedian;  ///< Stores Median of Average Duaration of sessions 
double bytesInitsMedian; ///<Stores Median of correlation between data and number of Initializations

size_t attemptLimit; ///<Stores maximum number of allowed attempts for forgotten password
size_t minFanout;///<Stores minimum number of IP's a bruteForcer must attack before it is captured as a distributed bruteForce Attack
size_t minFanin;///<Stores minimum number of times a IP is bruteForced before it is captured as a distributed BruteForce Attack

std::set <uint16_t> asnWhitelist;  ///<Stores ASN's to be whitelisted to prune out IM traffic for internal initiators 
std::set <uint16_t>:: iterator asnWhitelistItr;  
std::vector <std::string> asnWhitelistStrings; ///<Temporary vector to store ASN strings read from .conf file

int8_t typeInit = TEMPORARILY_UNKNOWN_INITIATOR;
uint16_t asn=0; ///<Stores ASN of current external IP being examined with an internal initiator

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
    numInitsMedian=0;
    avgDurationMedian.set(0,0);
    bytesInitsMedian=0;

	if (moduleState.conf.get(attemptLimit,
							 "attempt-limit",
							 "analysis_telnetBruteForcers")
			!= configuration::OK)
	{
		cerr << "analysis_telnetBruteForcers: missing or invalid attempt-limit"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(minFanin,
							 "min-fan-in",
							 "analysis_telnetBruteForcers")
			!= configuration::OK)
	{
		cerr << "analysis_telnetBruteForcers: missing or invalid min-fan-in"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(minFanout,
							 "min-fan-out",
							 "analysis_telnetBruteForcers")
			!= configuration::OK)
	{
		cerr << "analysis_telnetBruteForcers: missing or invalid min-fan-out"
			 << endl;
		return false;
	}

	string asn_white_list;
	if (moduleState.conf.get(asn_white_list,
							 "asn-white-list",
							 "analysis_telnetBruteForcers")
			!= configuration::OK)
	{
		cerr << "analysis_telnetBruteForcers: missing or invalid asn-white-list"
			 << endl;
		return false;
	}

    explodeString(asnWhitelistStrings, asn_white_list ," ");
    for( size_t asn = 0; asn < asnWhitelistStrings.size(); ++asn){
    	asnWhitelist.insert( boost::lexical_cast <uint16_t> (asnWhitelistStrings[asn]) );
    }
    asnWhitelistStrings.clear();
    return true;
 }


  /*
   * This function is called for every neoflow record. The hour variable is the
   * hour of the day (0 through 23) the record belongs to.
   */
 void aggregate(const FlowStats *flowStats, size_t) { 
    //filtering down to TCP connections on TELNET port
    if( isTCP(flowStats) && (isTELNETPort(flowStats->destinationPort()) || isTELNETPort(flowStats->sourcePort())) ){
	//Looking for three way TCP handshakes
	hostPair = makeTwoWayHostPair(flowStats, *localNetworks);
	//Getting initiator of three way TCP handshake
	typeInit = connectionInitiators->getInitiator(hostPair, flowStats->sourceIP());
	
	if(typeInit == TEMPORARILY_UNKNOWN_INITIATOR){ 
		connectionInitiators->aggregate(hostPair, flowStats);
		typeInit = connectionInitiators->getInitiator(hostPair, flowStats->sourceIP());
	}

	if(typeInit == INTERNAL_INITIATOR)
		asn = ipInformation->getASN(hostPair.externalIP);

	//filtering down to internal and external initiators
	if( (typeInit == EXTERNAL_INITIATOR && isTELNETPort(hostPair.internalPort)) || 
            (typeInit == INTERNAL_INITIATOR && asnWhitelist.find(asn) == asnWhitelist.end() && isTELNETPort(hostPair.externalPort))){
	        //Storing hostPair to specialized container of type bruteHostPair
		brutePair = hostPair;  

                //find brutePair in map
		bruteItr=bruteIPs.find(brutePair);
		
		//if hostPair was not seen before
		if(bruteItr == bruteIPs.end()){
			bruteItr=bruteIPs.insert(make_pair(brutePair, BruteForcers())).first;
		}

		bruteItr->second.typeInit = typeInit;
		bruteItr->second.numBytes += flowStats->numBytes();
		compareLessThan(bruteItr->second.firstStartTime, flowStats->startTime().seconds());
 	        compareGreaterThan(bruteItr->second.lastEndTime, flowStats->endTime().seconds());

		if(flowStats->tcpSYNs() && flowStats->firstSYNTime().seconds())
			bruteItr->second.startTime = flowStats->firstSYNTime();

		if(flowStats->tcpRSTs() || flowStats->tcpFINs()){
			++(bruteItr->second.numInits);
			bruteItr->second.endTime = flowStats->endTime();
			bruteItr->second.duration += bruteItr->second.endTime - bruteItr->second.startTime;
			bruteItr->second.avgDuration.set(bruteItr->second.duration.seconds()/bruteItr->second.numInits,
                                                         bruteItr->second.duration.microseconds()/bruteItr->second.numInits);
		}

		if(bruteItr->second.numBytes > 0 && bruteItr->second.numInits > 0 )
			bruteItr->second.bytesInitsRatio = static_cast<double>(bruteItr->second.numBytes)/bruteItr->second.numInits;
	}
    }
 }

  
  /*
   * This function is called once, after aggregate() has been called for each
   * neoflow record. Typical things to do here are to prepare the appropriate
   * SQL table using preparePGTable() commit any information gathered using the
   * PGBulkInserter class, and clear any state kept by the module. Returns the
   * number of SQL rows inserted upon success, or -1 upon failure.
   */
 
 int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {

	if(bruteIPs.empty())
		return 0;

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

        std::set <uint32_t>::iterator p2pNodesEnd(p2pNodes.end());

	PGBulkInserter pgBulkInserter(pg_conn.connection(), BRUTE_FORCERS_SCHEMA_NAME, date,
 	                                  flushSize, "%ud, %ud, %ud, %ud, %ud, %ud, %ud, %ud, %d");
 	Clock clock("Inserted", "rows");
 
	if (!preparePGTable(pg_conn.connection(), BRUTE_FORCERS_SCHEMA_NAME, date,
 	                    BRUTE_FORCERS_TABLE_SCHEMA, false)){
 	      return -1;
 	}
 	
    	uint32_t sourceIP, destinationIP;
 	uint16_t destinationPort;
	
	///Map that stores Fanout of attacker IP's 
	map <uint32_t, uint32_t> attackerIPCount;
	map <uint32_t, uint32_t> ::iterator attackerItr;

	///Map that stores Fanin of attacked IP's
	map <uint32_t, uint32_t> attackedIPCount;
	map <uint32_t, uint32_t> ::iterator attackedItr;

	map <BruteHostPair, BruteForcers>::iterator 
	bruteEnd(bruteIPs.end());
	map <BruteHostPair, BruteForcers>::iterator 
	bruteTemp;
	uint32_t attackerIP,attackedIP;

	clock.start();
	//Removing connections which timed out (FIN/RST flag was not seen)
	bruteItr= bruteIPs.begin();
	while(bruteItr != bruteEnd){
		if(bruteItr->second.numInits == 0){
			bruteTemp = bruteItr;
			++bruteItr;
			bruteIPs.erase(bruteTemp);
		}
		else{
			++bruteItr;
		}	
	}			  	
	
	if(bruteIPs.empty())
		return 0;

	//Gathering number of attempts,average duration and data-connection ratio for median analysis
	for(bruteItr = bruteIPs.begin(); bruteItr != bruteEnd; ++bruteItr ){
		//pushing in number of handshakes,average duration and data/initialization ratio in to vectors	
		numInits.push_back((*bruteItr).second.numInits);
		avgDuration.push_back((*bruteItr).second.avgDuration);
		bytesInitsRatio.push_back((*bruteItr).second.bytesInitsRatio);

		//Compete List of IP's <--Comment this section-->
		/*cout<<ntop(bruteItr -> first.internalIP)
		    <<"\t"<<ntop(bruteItr -> first.externalIP)<<"\t";
		if(bruteItr -> second.typeInit == INTERNAL_INITIATOR)
			cout<<"INTERNAL INITIATOR";
			else
			cout<<"EXTERNAL INITIATOR";
		cout<<"\t"<<bruteItr -> second.numInits
		    <<"\t"<<bruteItr -> second.avgDuration<<"secs"
		    <<"\t"<<bruteItr -> second.numBytes<<"bytes"
		    <<"\t"<<bruteItr -> second.bytesInitsRatio<<endl;*/
			
	}
	
	//Calculating Medain
	sort(numInits.begin(),numInits.end());
	sort(avgDuration.begin(),avgDuration.end());
	sort(bytesInitsRatio.begin(),bytesInitsRatio.end());
	numInitsMedian= *(numInits.begin()+numInits.size()/2);
	avgDurationMedian=*(avgDuration.begin()+avgDuration.size()/2);
	bytesInitsMedian=*(bytesInitsRatio.begin()+bytesInitsRatio.size()/2);

	//Median statistics <--Comment this section-->
	/*cout<<"Median of number of Initilizations:"<<numInitsMedian<<endl;
	cout<<"Median of average duration:"<<avgDurationMedian<<endl;
	cout<<"Median of ratio of Data and #Inits "<<bytesInitsMedian<<endl;
	cout<<endl;*/
	
	numInits.clear();
	avgDuration.clear();
   	bytesInitsRatio.clear();

	//Gathering Fanout and Fanin
	for(bruteItr = bruteIPs.begin(); bruteItr != bruteEnd; ++bruteItr ){
		if((bruteItr -> second.numInits > numInitsMedian && bruteItr -> second.bytesInitsRatio < bytesInitsMedian )|| 
		   (bruteItr -> second.avgDuration < avgDurationMedian)){
			if( bruteItr -> second.typeInit == EXTERNAL_INITIATOR ){
				attackerIP=bruteItr -> first.externalIP;
				attackedIP=bruteItr -> first.internalIP;
			}
			else{
				attackerIP=bruteItr -> first.internalIP;
				attackedIP=bruteItr -> first.externalIP;
			}
			attackerItr=attackerIPCount.find(attackerIP);
                	attackedItr=attackedIPCount.find(attackedIP);
		
			if(attackerItr == attackerIPCount.end())
				attackerIPCount.insert(make_pair(attackerIP,1));
			else
				++(attackerItr -> second);

			if(attackedItr == attackedIPCount.end())
				attackedIPCount.insert(make_pair(attackedIP,1));
			else
				++(attackedItr -> second);
		}
	} 
	
	///Final list of Brute IP's filtered on either of these two conditions:
	///(1)number of attempts > median of number of attempts and number of attempts > max number of  trials for forgotten password
	///(2)avg duration of connection attempts <= median of avg duration of connection attempts and Fanout >  minimum set number of IPs
	///(3)avg duration of connection attempts <= median of avg duration of connection attempts and Fanin >  minimum set number of IPs
	///Condition (1) catches bruteForcers repeatedly bruteForcing a particular target IP 
	///Condition (2) and (3) catch distributed brute Forcer attacks
	for(bruteItr = bruteIPs.begin(); bruteItr != bruteEnd; ++bruteItr ){

		if( bruteItr -> second.typeInit == EXTERNAL_INITIATOR ){
			attackerIP=bruteItr -> first.externalIP;
			attackedIP=bruteItr -> first.internalIP;
		}
		else{
			attackerIP=bruteItr -> first.internalIP;
			attackedIP=bruteItr -> first.externalIP;
		}
		attackerItr=attackerIPCount.find(attackerIP);
                attackedItr=attackedIPCount.find(attackedIP);
	
		if(p2pNodes.find(attackerIP) == p2pNodesEnd &&
                   ((bruteItr -> second.numInits > numInitsMedian && bruteItr -> second.bytesInitsRatio < bytesInitsMedian 
                                                                             && bruteItr -> second.numInits > attemptLimit)|| 
	   	    (bruteItr -> second.avgDuration < avgDurationMedian && attackerItr -> second > minFanout)||
 		    (bruteItr -> second.avgDuration < avgDurationMedian && attackedItr -> second > minFanin)) ){
			if(bruteItr -> second.typeInit == INTERNAL_INITIATOR){
				sourceIP = bruteItr -> first.internalIP;
 	          		destinationIP = bruteItr -> first.externalIP;
          		        destinationPort = bruteItr -> first.externalPort;
			}
			else{        		
				sourceIP = bruteItr -> first.externalIP;
                                destinationIP = bruteItr -> first.internalIP;
          		        destinationPort = bruteItr -> first.internalPort;
			}

			//Priting BruteForcers <--Comment this section-->
			/*cout << ntop(sourceIP) << " -----> " 
			     << ntop(destinationIP) <<":"
			     << destinationPort <<"\t"
			     << bruteItr -> second.numInits<<"\t"
			     << bruteItr -> second.avgDuration.seconds()<<"secs"<<"\t"
		   	     << bruteItr -> second.numBytes<<"bytes"<<"\t"
			     << bruteItr -> second.bytesInitsRatio<<endl;
			*/
			if (!pgBulkInserter.insert(NULL, sourceIP, destinationIP,
 	                                   destinationPort,
 	                                   bruteItr -> second.numInits,
                                           bruteItr->second.numBytes,
 	                                   bruteItr -> second.firstStartTime,
 	                                   bruteItr -> second.lastEndTime,
 	                                   ipInformation -> getASN(destinationIP),
 	                                   ipInformation -> getCountry(destinationIP))) {
 	       		return -1;
 	        	}
 	        	clock.incrementOperations();
 	        	 
		}
	}
        
 	if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
 	      return -1;
 	}
 	clock.stop();
	bruteIPs.clear();
	attackerIPCount.clear();
	attackedIPCount.clear();
 	return clock.operations();		
 }
}
