#include <iostream>
#include <vector>
#include <map>
#include <tr1/unordered_set>
#include <sys/endian.h>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "MiscHelpers.hpp"

#include "darkSpaceTarget.hpp"

#define DARK_SPACE_TARGETS_SCHEMA_NAME "DarkSpaceTargets"
#define DARK_SPACE_TARGETS_TABLE_SCHEMA "\"protocol\" SMALLINT NOT NULL, \
                                         \"internalIP\" uint32 NOT NULL, \
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
                                         PRIMARY KEY (\"protocol\", \"internalIP\", \
                                                      \"externalIP\", \"internalPort\", \
                                                      \"externalPort\")"

using namespace std;
using namespace tr1;

SharedState *mySharedState;
unordered_set <uint32_t> darkSpaceSources;
map <TwoWayHostPair, DarkSpaceTarget> darkSpaceTargets;
map <TwoWayHostPair, DarkSpaceTarget>::iterator darkSpaceTargetItr;

bool loadDarkSpaceSources(PGconn *postgreSQL, const string date) {
  PGresult *result;
  string query = "SELECT DISTINCT \"sourceIP\" FROM \"" \
                 DARK_SPACE_SOURCES_SCHEMA_NAME \
                 "\".\"" + date + '"';
  result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
                        1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return false;
  }
  for (int row = 0; row < PQntuples(result); ++row) {
    darkSpaceSources.insert(be32toh(*(uint32_t*)PQgetvalue(result, row, 0)));
  }
  return true;
}

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &) {
    mySharedState = &sharedState;
    if (!loadDarkSpaceSources(sharedState.postgreSQL, sharedState.date)) {
      return false;
    }
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    static TwoWayHostPair hostPair;
    if (mySharedState -> liveIPs -> find(flowStats -> destinationIP()) != mySharedState -> liveIPs -> end() &&
        darkSpaceSources.find(flowStats -> sourceIP()) != darkSpaceSources.end()) {
      hostPair = makeTwoWayHostPair(flowStats, *(mySharedState -> localNetworks));
      darkSpaceTargetItr = darkSpaceTargets.find(hostPair);
      if (darkSpaceTargetItr == darkSpaceTargets.end()) {
        darkSpaceTargetItr = darkSpaceTargets.insert(make_pair(hostPair, DarkSpaceTarget())).first;
      }
      if (darkSpaceTargetItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
        mySharedState -> connectionInitiators -> aggregate(hostPair, flowStats);
        darkSpaceTargetItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair,
                                                                                                       flowStats -> sourceIP());
      }
      darkSpaceTargetItr -> second.numBytes += flowStats -> numBytes();
      darkSpaceTargetItr -> second.numPackets += flowStats -> numPackets();
      compareLessThan(darkSpaceTargetItr -> second.minPacketSize,
                      flowStats -> minPacketSize());
      compareGreaterThan(darkSpaceTargetItr -> second.maxPacketSize,
                         flowStats -> maxPacketSize());
      compareLessThan(darkSpaceTargetItr -> second.startTime,
                      flowStats -> startTime().seconds());
      compareGreaterThan(darkSpaceTargetItr -> second.endTime,
                         flowStats -> endTime().seconds());
      accumulateContent(darkSpaceTargetItr -> second.content,
                        flowStats);
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    Clock clock("Inserted", "rows");
    if (!preparePGTable(pg_conn.connection(), DARK_SPACE_TARGETS_SCHEMA_NAME, date,
                        DARK_SPACE_TARGETS_TABLE_SCHEMA)) {
      return -1;
    }
    PGBulkInserter pgBulkInserter(pg_conn.connection(), DARK_SPACE_TARGETS_SCHEMA_NAME,
                                  date, flushSize, "%d, %ud, %ud, %ud, %ud, " \
                                  "%ud, %ud, %ud, %ud, %ud, %ud, %ud, %ud, " \
                                  "%d, %Vud");
    cout << "Updating PostgreSQL database with dark space targets" << endl;
    clock.start();
    for (map <TwoWayHostPair, DarkSpaceTarget>::iterator darkSpaceTargetItr = darkSpaceTargets.begin();
         darkSpaceTargetItr != darkSpaceTargets.end(); ++darkSpaceTargetItr) {
      if (darkSpaceTargetItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
        darkSpaceTargetItr -> second.initiator = PERMANENTLY_UNKNOWN_INITIATOR;
      }
      if (!pgBulkInserter.insert(NULL, darkSpaceTargetItr -> first.protocol,
                                 darkSpaceTargetItr -> first.internalIP,
                                 darkSpaceTargetItr -> first.externalIP,
                                 darkSpaceTargetItr -> first.internalPort,
                                 darkSpaceTargetItr -> first.externalPort,
                                 darkSpaceTargetItr -> second.initiator,
                                 darkSpaceTargetItr -> second.numBytes,
                                 darkSpaceTargetItr -> second.numPackets,
                                 darkSpaceTargetItr -> second.minPacketSize,
                                 darkSpaceTargetItr -> second.maxPacketSize,
                                 darkSpaceTargetItr -> second.startTime,
                                 darkSpaceTargetItr -> second.endTime,
                                 mySharedState -> ipInformation -> getASN(darkSpaceTargetItr -> first.externalIP),
                                 mySharedState -> ipInformation -> getCountry(darkSpaceTargetItr -> first.externalIP),
                                 (void*)&(darkSpaceTargetItr -> second.content))) {
        return -1;
      }
      clock.incrementOperations();
    }
    if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
      return -1;
    }
    clock.stop();
    darkSpaceSources.clear();
    darkSpaceTargets.clear();
    return clock.operations();
  }
}
