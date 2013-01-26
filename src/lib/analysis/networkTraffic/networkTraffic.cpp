#include <iostream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"

#define NETWORK_TRAFFIC_SCHEMA_NAME "NetworkTraffic"
#define NETWORK_TRAFFIC_TABLE_SCHEMA "\"numLiveIPs\" uint32 NOT NULL, \
                                      \"numFlows\" uint64 NOT NULL, \
                                      \"numPackets\" uint64 NOT NULL, \
                                      \"numBytes\" uint64 NOT NULL"

using namespace std;
using namespace tr1;

unordered_map <uint32_t, string> *liveIPs;

uint64_t numFlows, numPackets, numBytes;

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &) {
    liveIPs = sharedState.liveIPs;
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    ++numFlows;
    numPackets += flowStats -> numPackets();
    numBytes += flowStats -> numBytes();
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &, const char *date) {
    if (!preparePGTable(pg_conn.connection(), NETWORK_TRAFFIC_SCHEMA_NAME, date,
                        NETWORK_TRAFFIC_TABLE_SCHEMA))
	{
	  cerr << "networkTraffic: unable to prepare table." << endl;
      return -1;
    }
    
    if (!insertPGRow(pg_conn.connection(), NETWORK_TRAFFIC_SCHEMA_NAME, date,
                     "%ud, %ul, %ul, %ul", liveIPs -> size(), numFlows,
                     numPackets, numBytes))
	{
	  cerr << "networkTraffic: unable to insert network traffic info." << endl;
      return -1;
    }
    return 1;
  }
}
