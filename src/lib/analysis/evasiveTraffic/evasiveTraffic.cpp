#include <iostream>
#include <vector>
#include <map>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "MiscHelpers.hpp"

#include "evasiveTraffic.hpp"

#define EVASIVE_TRAFFIC_SCHEMA_NAME "EvasiveTraffic"
#define EVASIVE_TRAFFIC_TABLE_SCHEMA "\"protocol\" SMALLINT NOT NULL, \
                                      \"internalIP\" uint32 NOT NULL, \
                                      \"externalIP\" uint32 NOT NULL, \
                                      \"internalPort\" uint16 NOT NULL, \
                                      \"externalPort\" uint16 NOT NULL, \
                                      \"initiator\" SMALLINT NOT NULL, \
                                      \"numBytes\" uint32 NOT NULL, \
                                      \"numPackets\" uint32 NOT NULL, \
                                      \"minPacketSize\" uint16 NOT NULL, \
                                      \"maxPacketSize\" uint16 NOT NULL, \
                                      \"numFrags\" uint32 NOT NULL, \
                                      \"minTTL\" SMALLINT NOT NULL, \
                                      \"maxTTL\" SMALLINT NOT NULL, \
                                      \"startTime\" uint32 NOT NULL, \
                                      \"endTime\" uint32 NOT NULL, \
                                      \"asNumber\" uint16 NOT NULL, \
                                      \"countryNumber\" SMALLINT NOT NULL, \
                                      \"content\" uint32[] NOT NULL, \
                                      PRIMARY KEY (\"protocol\", \"internalIP\", \
                                                   \"externalIP\", \"internalPort\", \
                                                   \"externalPort\")"

using namespace std;
using namespace tr1;

SharedState *mySharedState;
size_t minimumTTL;
map <TwoWayHostPair, EvasiveTraffic> evasiveTraffic;

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &moduleState) {
    mySharedState = &sharedState;
	if (moduleState.conf.get(minimumTTL, "min-ttl", "analysis_evasiveTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_evasiveTraffic: missing or invalid min-ttl" << endl;
		return false;
	}
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    static TwoWayHostPair hostPair;
    static map <TwoWayHostPair, EvasiveTraffic>::iterator evasiveTrafficItr;
    if ((!isUDP(flowStats) && flowStats -> numFrags()) ||
        (flowStats -> minTTL() < minimumTTL && flowStats -> maxTTL() < minimumTTL)) {
      hostPair = makeTwoWayHostPair(flowStats, *(mySharedState -> localNetworks));
      if (mySharedState -> liveIPs -> find(hostPair.internalIP) != mySharedState -> liveIPs -> end()) {
        evasiveTrafficItr = evasiveTraffic.find(hostPair);
        if (evasiveTrafficItr == evasiveTraffic.end()) {
          evasiveTrafficItr = evasiveTraffic.insert(make_pair(hostPair, EvasiveTraffic())).first;
        }
        if (evasiveTrafficItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
          mySharedState -> connectionInitiators -> aggregate(hostPair, flowStats);
          evasiveTrafficItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair,
                                                                                                        flowStats -> sourceIP());
        }
        evasiveTrafficItr -> second.numBytes += flowStats -> numBytes();
        evasiveTrafficItr -> second.numPackets += flowStats -> numPackets();
        compareLessThan(evasiveTrafficItr -> second.minPacketSize,
                        flowStats -> minPacketSize());
        compareGreaterThan(evasiveTrafficItr -> second.maxPacketSize,
                           flowStats -> maxPacketSize());
        evasiveTrafficItr -> second.numFrags += flowStats -> numFrags();
        compareLessThan(evasiveTrafficItr -> second.minTTL,
                        flowStats -> minTTL());
        compareGreaterThan(evasiveTrafficItr -> second.maxTTL,
                           flowStats -> maxTTL());
        compareLessThan(evasiveTrafficItr -> second.startTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(evasiveTrafficItr -> second.endTime,
                           flowStats -> endTime().seconds());
        accumulateContent(evasiveTrafficItr -> second.content, flowStats);
      }
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    Clock clock("Inserted", "rows");
    if (!preparePGTable(pg_conn.connection(), EVASIVE_TRAFFIC_SCHEMA_NAME, date,
                        EVASIVE_TRAFFIC_TABLE_SCHEMA)) {
      return -1;
    }
    PGBulkInserter pgBulkInserter(pg_conn.connection(), EVASIVE_TRAFFIC_SCHEMA_NAME,
                                  date, flushSize, "%d, %ud, %ud, %ud, %ud, " \
                                  "%d, %ud, %ud, %ud, %ud, %ud, %ud, %ud, " \
                                  "%ud, %ud, %ud, %d, %Vud");
    cout << "Updating PostgreSQL database with evasive traffic" << endl;
    clock.start();
    for (map <TwoWayHostPair, EvasiveTraffic>::iterator evasiveTrafficItr = evasiveTraffic.begin();
         evasiveTrafficItr != evasiveTraffic.end(); ++evasiveTrafficItr) {
      if (evasiveTrafficItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
        evasiveTrafficItr -> second.initiator = PERMANENTLY_UNKNOWN_INITIATOR;
      }
      if (!pgBulkInserter.insert(NULL, evasiveTrafficItr -> first.protocol,
                                 evasiveTrafficItr -> first.internalIP,
                                 evasiveTrafficItr -> first.externalIP,
                                 evasiveTrafficItr -> first.internalPort,
                                 evasiveTrafficItr -> first.externalPort,
                                 evasiveTrafficItr -> second.initiator,
                                 evasiveTrafficItr -> second.numBytes,
                                 evasiveTrafficItr -> second.numPackets,
                                 evasiveTrafficItr -> second.minPacketSize,
                                 evasiveTrafficItr -> second.maxPacketSize,
                                 evasiveTrafficItr -> second.numFrags,
                                 evasiveTrafficItr -> second.minTTL,
                                 evasiveTrafficItr -> second.maxTTL,
                                 evasiveTrafficItr -> second.startTime,
                                 evasiveTrafficItr -> second.endTime,
                                 mySharedState -> ipInformation -> getASN(evasiveTrafficItr -> first.externalIP),
                                 mySharedState -> ipInformation -> getCountry(evasiveTrafficItr -> first.externalIP),
                                 (void*)&(evasiveTrafficItr -> second.content))) {
        return -1;
      }
      clock.incrementOperations();
    }
    if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
      return -1;
    }
    clock.stop();
    evasiveTraffic.clear();
    return clock.operations();
  }
}
