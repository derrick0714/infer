#include <iostream>
#include <sstream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"

#define TOP_PORTS_SCHEMA_NAME "TopPorts"
#define TOP_PORTS_TABLE_SCHEMA "\"ip\" uint32 NOT NULL, \
                                \"topInboundPorts\" uint16[] NOT NULL, \
                                \"topInboundTraffic\" uint64[] NOT NULL, \
                                \"topOutboundPorts\" uint16[] NOT NULL, \
                                \"topOutboundTraffic\" uint64[] NOT NULL"

using namespace std;
using namespace tr1;

class TopPorts {
  public:
    unordered_map <uint16_t, uint64_t> inboundBytes;
    unordered_map <uint16_t, uint64_t> outboundBytes;
};

unordered_map <uint32_t, string> *liveIPs;
unordered_map <uint32_t, TopPorts> topPorts;

inline void getPorts(multimap <uint64_t, uint16_t> &container,
                     vector <uint16_t> &ports) {
  ports.clear();
  for (multimap <uint64_t, uint16_t>::iterator itr = container.begin();
       itr != container.end(); ++itr) {
    ports.push_back(itr -> second);
  }
  reverse(ports.begin(), ports.end());
}

inline void getBytes(multimap <uint64_t, uint16_t> &container,
                     vector <uint64_t> &bytes) {
  bytes.clear();
  for (multimap <uint64_t, uint16_t>::iterator itr = container.begin();
       itr != container.end(); ++itr) {
    bytes.push_back(itr -> first);
  }
  reverse(bytes.begin(), bytes.end());
}

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &) {
    liveIPs = sharedState.liveIPs;
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    static TwoWayHostPair hostPair;
    static unordered_map <uint32_t, TopPorts>::iterator topPortItr;
    static unordered_map <uint16_t, uint64_t>::iterator portItr;
    if (liveIPs -> find(flowStats -> destinationIP()) != liveIPs -> end()) {
      topPortItr = topPorts.find(flowStats -> destinationIP());
      if (topPortItr == topPorts.end()) {
        topPortItr = topPorts.insert(make_pair(flowStats -> destinationIP(), TopPorts())).first;
      }
      portItr = topPortItr -> second.inboundBytes.find(flowStats -> destinationPort());
      if (portItr == topPortItr -> second.inboundBytes.end()) {
        portItr = topPortItr -> second.inboundBytes.insert(make_pair(flowStats -> destinationPort(), 0)).first;
      }
      portItr -> second += flowStats -> numBytes();
    }
    if (liveIPs -> find(flowStats -> sourceIP()) != liveIPs -> end()) {
      topPortItr = topPorts.find(flowStats -> sourceIP());
      if (topPortItr == topPorts.end()) {
        topPortItr = topPorts.insert(make_pair(flowStats -> sourceIP(), TopPorts())).first;
      }
      portItr = topPortItr -> second.outboundBytes.find(flowStats -> destinationPort());
      if (portItr == topPortItr -> second.outboundBytes.end()) {
        portItr = topPortItr -> second.outboundBytes.insert(make_pair(flowStats -> destinationPort(), 0)).first;
      }
      portItr -> second += flowStats -> numBytes();
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    multimap <uint64_t, uint16_t> orderedInboundPortTraffic, orderedOutboundPortTraffic;
    vector <uint16_t> inboundPorts, outboundPorts;
    vector <uint64_t> inboundBytes, outboundBytes;
    Clock clock("Inserted", "rows");
    if (!preparePGTable(pg_conn.connection(), TOP_PORTS_SCHEMA_NAME, date,
                        TOP_PORTS_TABLE_SCHEMA)) {
      return -1;                      
    }
    PGBulkInserter pgBulkInserter(pg_conn.connection(), TOP_PORTS_SCHEMA_NAME, date,
                                  flushSize, "%ud, %Vuh, %Vul, %Vuh, %Vul");
    cout << "Updating PostgreSQL database with top ports" << endl;
    clock.start();
    for (unordered_map <uint32_t, TopPorts>::iterator internalIPItr = topPorts.begin();
         internalIPItr != topPorts.end(); ++internalIPItr) {
      for (unordered_map <uint16_t, uint64_t>::iterator inboundPortTrafficItr = internalIPItr -> second.inboundBytes.begin();
           inboundPortTrafficItr != internalIPItr -> second.inboundBytes.end(); ++inboundPortTrafficItr) {
        orderedInboundPortTraffic.insert(make_pair(inboundPortTrafficItr -> second,
                                                   inboundPortTrafficItr -> first));
      }
      for (unordered_map <uint16_t, uint64_t>::iterator outboundPortTrafficItr = internalIPItr -> second.outboundBytes.begin();
           outboundPortTrafficItr != internalIPItr -> second.outboundBytes.end(); ++outboundPortTrafficItr) {
        orderedOutboundPortTraffic.insert(make_pair(outboundPortTrafficItr -> second,
                                                    outboundPortTrafficItr -> first));
      }
      while (orderedInboundPortTraffic.size() > 10) {
        orderedInboundPortTraffic.erase(orderedInboundPortTraffic.begin());
      }
      while (orderedOutboundPortTraffic.size() > 10) {
        orderedOutboundPortTraffic.erase(orderedOutboundPortTraffic.begin());
      }
      getPorts(orderedInboundPortTraffic, inboundPorts);
      getBytes(orderedInboundPortTraffic, inboundBytes);
      getPorts(orderedOutboundPortTraffic, outboundPorts);
      getBytes(orderedOutboundPortTraffic, outboundBytes);
      if (!pgBulkInserter.insert(NULL, internalIPItr -> first,
                                 (void*)&inboundPorts, (void*)&inboundBytes,
                                 (void*)&outboundPorts,
                                 (void*)&outboundBytes)) {
        return -1;
      }
      clock.incrementOperations();
      orderedInboundPortTraffic.clear();
      orderedOutboundPortTraffic.clear();
    }
    if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
      return -1;
    }
    clock.stop();
    topPorts.clear();
    return clock.operations();
  }
}
