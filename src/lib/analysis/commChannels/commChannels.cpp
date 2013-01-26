#include <vector>
#include <map>
#include <tr1/unordered_set>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "dns.h"
#include "MiscHelpers.hpp"

#include "commChannel.hpp"

#define COMM_CHANNELS_TABLE_SCHEMA "\"internalIP\" uint32 NOT NULL, \
									\"externalIP\" uint32 NOT NULL, \
									\"internalPort\" uint16 NOT NULL, \
									\"externalPort\" uint16 NOT NULL, \
									\"initiator\" SMALLINT NOT NULL, \
									\"numBytes\" uint32 NOT NULL, \
									\"numPackets\" uint32 NOT NULL, \
									\"minPacketSize\" uint16 NOT NULL, \
									\"maxPacketSize\" uint16 NOT NULL, \
									\"startTime\" uint32 NOT NULL, \
									\"endTime\" uint32 NOT NULL, \
									\"asNumber\" uint16 NOT NULL, \
									\"countryNumber\" SMALLINT NOT NULL, \
									\"content\" uint32[] NOT NULL, \
									\"external_ip_fanin\" uint32, \
									PRIMARY KEY (\"internalIP\", \
												 \"externalIP\", \
												 \"internalPort\", \
												 \"externalPort\")"

#define ROLES_TABLE_SCHEMA	"\"ip\" uint32 NOT NULL, \
							 \"role\" uint16 NOT NULL, \
							 \"dangerous\" SMALLINT NOT NULL, \
							 \"numHosts\" uint32 NOT NULL, \
							 \"numBytes\" uint64 NOT NULL, \
							 \"startTime\" uint32 NOT NULL, \
							 \"endTime\" uint32 NOT NULL, \
							 PRIMARY KEY (\"ip\", \"role\")"

using namespace std;
using namespace tr1;

SharedState *mySharedState;
size_t maxCommChannels;
unordered_set <uint32_t> dangerousIPs;
map <TwoWayHostPair, CommChannel> commChannels;

/* Stores potential communication channels in PostgreSQL. */
int commitCommChannels(PGconn *postgreSQL, size_t &flushSize, const char *date)
{
	Clock clock("Inserted", "rows");
	PGBulkInserter pgBulkInserter(postgreSQL, COMM_CHANNELS_SCHEMA_NAME,
								  date, flushSize, "%ud, %ud, %ud, %ud, %ud, "
								  "%ud, %ud, %ud, %ud, %ud, "
								  "%ud, %ud, %d, %Vud");
	vector <uint32_t> content(FlowStats::CONTENT_TYPES);
	size_t index;
	if (!preparePGTable(postgreSQL, COMM_CHANNELS_SCHEMA_NAME, date,
						COMM_CHANNELS_TABLE_SCHEMA))
	{
		return -1;
	}
	cout << "Updating PostgreSQL database with communication channels" << endl;
	clock.start();
	for (map <TwoWayHostPair, CommChannel>::iterator commChanItr(
			commChannels.begin());
		 commChanItr != commChannels.end();
		 ++commChanItr)
	{
		if (commChanItr->second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
			commChanItr->second.initiator = PERMANENTLY_UNKNOWN_INITIATOR;
		}
		for (index = 0; index < FlowStats::CONTENT_TYPES; ++index) {
			content[index] = commChanItr->second.content[index];
		}
		if (!pgBulkInserter.insert(NULL, commChanItr->first.internalIP,
								  commChanItr->first.externalIP,
								  commChanItr->first.internalPort,
								  commChanItr->first.externalPort,
								  commChanItr->second.initiator,
								  commChanItr->second.numBytes,
								  commChanItr->second.numPackets,
								  commChanItr->second.minPacketSize,
								  commChanItr->second.maxPacketSize,
								  commChanItr->second.startTime,
								  commChanItr->second.endTime,
								  mySharedState->ipInformation->getASN(
										commChanItr->first.externalIP),
								  mySharedState->ipInformation->getCountry(
										commChanItr->first.externalIP),
								  (void*)&content))
		{
			return -1;
		}
		clock.incrementOperations();
	}
	if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
		return -1;
	}
	clock.stop();
	commChannels.clear();
	return clock.operations();
}

/* Stores roles in PostgreSQL. */
int commitRoles(PGconn *postgreSQL, size_t &flushSize, const char *date) {
	Clock clock("Inserted", "rows");
	PGBulkInserter pgBulkInserter(postgreSQL,
								  ROLES_SCHEMA_NAME,
								  date,
								  flushSize,
								  "%ud, %ud, %d, %ud, %ul, %ud, %ud");
	if (!preparePGTable(postgreSQL, ROLES_SCHEMA_NAME, date,
						ROLES_TABLE_SCHEMA)) {
		return -1;
	}
	cout << "Updating PostgreSQL database with roles" << endl;
	clock.start();
	for (unordered_multimap <uint32_t, Role>::iterator roleItr(
			mySharedState->roles->begin());
		 roleItr != mySharedState->roles->end();
		 ++roleItr)
	{
		if (!pgBulkInserter.insert(NULL,
								   roleItr->first,
								   roleItr->second._role,
								   roleItr->second._dangerous,
								   roleItr->second._numHosts,
								   roleItr->second._numBytes,
								   roleItr->second._startTime,
								   roleItr->second._endTime))
		{
			return -1;
		}
		if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
			return -1;
		}
		clock.incrementOperations();
	}
	if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
		return -1;
	}
	clock.stop();
	mySharedState->roles->clear();
	return clock.operations();
}

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		mySharedState = &sharedState;
		if (moduleState.conf.get(maxCommChannels,
								 "max-comm-channels",
								 "analysis_commChannels")
						!= configuration::OK)
		{
			cerr << "analysis_commChannels: missing or invalid max-flows"
				 << endl;
			return false;
		}
		for (unordered_multimap <uint32_t, Role>::iterator hostItr(
				sharedState.roles->begin());
			 hostItr != sharedState.roles->end();
			 ++hostItr)
		{
			if (hostItr->second._dangerous) {
				dangerousIPs.insert(hostItr->first);
			}
		}
		return true;
	}

	void aggregate(const FlowStats *flowStats, size_t) {
		static TwoWayHostPair hostPair;
		static map <TwoWayHostPair, CommChannel>::iterator commChanItr;
		if (isTCP(flowStats)) {
			hostPair = makeTwoWayHostPair(flowStats,
										  *(mySharedState->localNetworks));
			if (dangerousIPs.find(hostPair.internalIP) != dangerousIPs.end()) {
				commChanItr = commChannels.find(hostPair);
				if (commChanItr == commChannels.end()) {
					commChanItr = commChannels.insert(
						make_pair(hostPair, CommChannel())).first;
				}
				if (commChanItr->second.initiator
						== TEMPORARILY_UNKNOWN_INITIATOR)
				{
					mySharedState->connectionInitiators->aggregate(hostPair,
																   flowStats);
					commChanItr->second.initiator =
						mySharedState->connectionInitiators->getInitiator(
							hostPair, flowStats->sourceIP());
				}
				++(commChanItr->second.numFlows);
				commChanItr->second.numBytes += flowStats->numBytes();
				commChanItr->second.numPackets += flowStats->numPackets();
				compareLessThan(commChanItr->second.minPacketSize,
								flowStats->minPacketSize());
				compareGreaterThan(commChanItr->second.maxPacketSize,
								   flowStats->maxPacketSize());
				compareLessThan(commChanItr->second.startTime,
								flowStats->startTime().seconds());
				compareGreaterThan(commChanItr->second.endTime,
								   flowStats->endTime().seconds());
				accumulateContent(commChanItr->second.content,
								  flowStats);
			}
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &flushSize,
			   const char *date)
	{
		int numCommChannels, numRoles;
		numCommChannels = commitCommChannels(pg_conn.connection(),
											 flushSize,
											 date);
		if (numCommChannels == -1) {
			return -1;
		}
		numRoles = commitRoles(pg_conn.connection(), flushSize, date);
		if (numRoles == -1) {
			return -1;
		}
		return (numCommChannels + numRoles);
	}
}
