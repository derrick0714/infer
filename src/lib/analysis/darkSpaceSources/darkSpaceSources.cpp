#include <iostream>
#include <vector>
#include <map>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "MiscHelpers.hpp"

#define DARK_SPACE_SOURCES_TABLE_SCHEMA "\"protocol\" SMALLINT NOT NULL, \
                                         \"sourceIP\" uint32 NOT NULL, \
                                         \"destinationIP\" uint32 NOT NULL, \
                                         \"sourcePort\" uint16 NOT NULL, \
                                         \"destinationPort\" uint16 NOT NULL, \
                                         \"numBytes\" uint32 NOT NULL, \
                                         \"numPackets\" uint32 NOT NULL, \
                                         \"minPacketSize\" uint16 NOT NULL, \
                                         \"maxPacketSize\" uint16 NOT NULL, \
                                         \"startTime\" uint32 NOT NULL, \
                                         \"endTime\" uint32 NOT NULL, \
                                         \"asNumber\" uint16 NOT NULL, \
                                         \"countryNumber\" SMALLINT NOT NULL, \
                                         \"content\" uint32[] NOT NULL, \
                                         PRIMARY KEY (\"protocol\", \"sourceIP\", \
                                                      \"destinationIP\", \"sourcePort\", \
                                                      \"destinationPort\")"

using namespace std;
using namespace tr1;

class DarkSpaceSource {
  public:
    uint32_t numBytes;
    uint32_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    vector <uint32_t> content;
    DarkSpaceSource();
};

inline DarkSpaceSource::DarkSpaceSource() {
  numBytes = 0;
  numPackets = 0;
  minPacketSize = numeric_limits <uint16_t>::max();;
  maxPacketSize = 0;
  startTime = numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

vector <pair <uint32_t, uint32_t> > *localNetworks;
unordered_map <uint32_t, string> *liveIPs;
IPInformation *ipInformation;

OneWayHostPair hostPair;
map <OneWayHostPair, DarkSpaceSource> darkSpaceSources;
map <OneWayHostPair, DarkSpaceSource>::iterator darkSpaceSourceItr;

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &) {
    localNetworks = sharedState.localNetworks;
    liveIPs = sharedState.liveIPs;
    ipInformation = sharedState.ipInformation;
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    if (isInternal(flowStats -> destinationIP(), *localNetworks) &&
        liveIPs -> find(flowStats -> destinationIP()) == liveIPs -> end()) {
      hostPair = makeOneWayHostPair(flowStats);
      darkSpaceSourceItr = darkSpaceSources.find(hostPair);
      if (darkSpaceSourceItr == darkSpaceSources.end()) {
        darkSpaceSourceItr = darkSpaceSources.insert(make_pair(hostPair, DarkSpaceSource())).first;
      }
      darkSpaceSourceItr -> second.numBytes += flowStats -> numBytes();
      darkSpaceSourceItr -> second.numPackets += flowStats -> numPackets();
      compareLessThan(darkSpaceSourceItr -> second.minPacketSize,
                      flowStats -> minPacketSize());
      compareGreaterThan(darkSpaceSourceItr -> second.maxPacketSize,
                         flowStats -> maxPacketSize());
      compareLessThan(darkSpaceSourceItr -> second.startTime,
                      flowStats -> startTime().seconds());
      compareGreaterThan(darkSpaceSourceItr -> second.endTime,
                         flowStats -> endTime().seconds());
      accumulateContent(darkSpaceSourceItr -> second.content,
                        flowStats);
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    Clock clock("Inserted", "rows");
    if (!preparePGTable(pg_conn.connection(), DARK_SPACE_SOURCES_SCHEMA_NAME, date,
                        DARK_SPACE_SOURCES_TABLE_SCHEMA)) {
      return -1;
    }
    PGBulkInserter pgBulkInserter(pg_conn.connection(), DARK_SPACE_SOURCES_SCHEMA_NAME,
                                  date, flushSize, "%d, %ud, %ud, %ud, %ud, " \
                                  "%ud, %ud, %ud, %ud, %ud, %ud, %ud, %d, " \
                                  "%Vud");
    cout << "Updating PostgreSQL database with dark space sources" << endl;
    clock.start();
    for (map <OneWayHostPair, DarkSpaceSource>::iterator darkSpaceSourceItr = darkSpaceSources.begin();
         darkSpaceSourceItr != darkSpaceSources.end(); ++darkSpaceSourceItr) {
      if (!pgBulkInserter.insert(NULL, darkSpaceSourceItr -> first.protocol,
                                 darkSpaceSourceItr -> first.sourceIP,
                                 darkSpaceSourceItr -> first.destinationIP,
                                 darkSpaceSourceItr -> first.sourcePort,
                                 darkSpaceSourceItr -> first.destinationPort,
                                 darkSpaceSourceItr -> second.numBytes,
                                 darkSpaceSourceItr -> second.numPackets,
                                 darkSpaceSourceItr -> second.minPacketSize,
                                 darkSpaceSourceItr -> second.maxPacketSize,
                                 darkSpaceSourceItr -> second.startTime,
                                 darkSpaceSourceItr -> second.endTime,
                                 ipInformation -> getASN(darkSpaceSourceItr -> first.sourceIP),
                                 ipInformation -> getCountry(darkSpaceSourceItr -> first.sourceIP),
                                 (void*)&darkSpaceSourceItr -> second.content)) {
        return -1;
      }
      clock.incrementOperations();
    }
    if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
      return -1;
    }
    clock.stop();
    darkSpaceSources.clear();
    return clock.operations();
  }
}
