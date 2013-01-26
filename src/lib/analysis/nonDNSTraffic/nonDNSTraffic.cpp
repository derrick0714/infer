#include <iostream>
#include <vector>
#include <map>
#include <sys/endian.h>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "address.h"
#include "MiscHelpers.hpp"

#include "nonDNSTraffic.hpp"

#define MULTIMEDIA_FLOW 	1
#define UNCLASSIFIED_FLOW	2
#define ENCRYPTED_FLOW		3

#define NON_DNS_TRAFFIC_TABLE_SCHEMA "\"protocol\" SMALLINT NOT NULL, \
                                      \"internalIP\" uint32 NOT NULL, \
                                      \"externalIP\" uint32 NOT NULL, \
                                      \"internalPort\" uint16 NOT NULL, \
                                      \"externalPort\" uint16 NOT NULL, \
                                      \"numBytes\" uint64 NOT NULL, \
                                      \"numPackets\" uint64 NOT NULL, \
                                      \"minPacketSize\" uint16 NOT NULL, \
                                      \"maxPacketSize\" uint16 NOT NULL, \
                                      \"content\" uint32[] NOT NULL, \
                                      \"startTime\" uint32 NOT NULL, \
                                      \"endTime\" uint32 NOT NULL, \
                                      \"asNumber\" uint16 NOT NULL, \
                                      \"countryNumber\" SMALLINT NOT NULL, \
                                      \"type\" SMALLINT NOT NULL, \
                                      PRIMARY KEY (\"protocol\", \"internalIP\", \
                                                   \"externalIP\", \"internalPort\", \
                                                   \"externalPort\")"

using namespace std;
using namespace tr1;

SharedState *mySharedState;
size_t minFanOut, minTraffic;
vector <pair <uint32_t, uint32_t> > whiteList;
map <TwoWayHostPair, NonDNSTraffic> nonDNSTraffic;

bool getWhiteList(PGconn *postgreSQL) {
  ostringstream query;
  PGresult *result;
  query << "SELECT \"ipBlock\" FROM \"Maps\".\"nonDNSTrafficWhiteList\"";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    PQclear(result);
    return false;
  }
  for (int index = 0; index < PQntuples(result); ++index) {
    whiteList.push_back(cidrToRange((char*)PQgetvalue(result,index,0)));
  }
  PQclear(result);
  return true;
}

int commitNonDNSTraffic(PGconn *postgreSQL, size_t &flushSize,
                        const char *date) {
  uint8_t type;
  Clock clock("Inserted", "rows");
  if (!preparePGTable(postgreSQL, NON_DNS_TRAFFIC_SCHEMA_NAME, date,
                      NON_DNS_TRAFFIC_TABLE_SCHEMA)) {
    return -1;
  }
  PGBulkInserter pgBulkInserter(postgreSQL, NON_DNS_TRAFFIC_SCHEMA_NAME,
                                date, flushSize, "%d, %ud, %ud, %ud, %ud, " \
                                "%ul, %ul, %ud, %ud, %Vud, %ud, %ud, %ud, " \
                                "%d, %ud");
  cout << endl << "Updating PostgreSQL database with non-DNS traffic" << endl;
  clock.start();
  for (map <TwoWayHostPair, NonDNSTraffic>::iterator hostPairItr = nonDNSTraffic.begin();
       hostPairItr != nonDNSTraffic.end(); ++hostPairItr) {
    if (hostPairItr -> second.status == BAD_STATUS) {
      if ((hostPairItr -> second.content[FlowStats::WAV_AUDIO_TYPE] + hostPairItr -> second.content[FlowStats::JPEG_IMAGE_TYPE] +
           hostPairItr -> second.content[FlowStats::MP3_AUDIO_TYPE] + hostPairItr -> second.content[FlowStats::MPEG_VIDEO_TYPE]) >
          (hostPairItr -> second.content[FlowStats::PLAINTEXT_TYPE] + hostPairItr -> second.content[FlowStats::BMP_IMAGE_TYPE] +
           hostPairItr -> second.content[FlowStats::COMPRESSED_TYPE] + hostPairItr -> second.content[FlowStats::ENCRYPTED_TYPE])) {
        type = MULTIMEDIA_FLOW;
      }
      else {
        if ((hostPairItr -> second.content[FlowStats::COMPRESSED_TYPE] + hostPairItr -> second.content[FlowStats::ENCRYPTED_TYPE]) >
            (hostPairItr -> second.content[FlowStats::PLAINTEXT_TYPE] + hostPairItr -> second.content[FlowStats::BMP_IMAGE_TYPE] +
             hostPairItr -> second.content[FlowStats::WAV_AUDIO_TYPE] + hostPairItr -> second.content[FlowStats::JPEG_IMAGE_TYPE] +
             hostPairItr -> second.content[FlowStats::MP3_AUDIO_TYPE] + hostPairItr -> second.content[FlowStats::MPEG_VIDEO_TYPE])) {
          type = ENCRYPTED_FLOW;
        }
        else {
          type = UNCLASSIFIED_FLOW;
        }
      }
      if (!pgBulkInserter.insert(NULL, hostPairItr -> first.protocol,
                                 hostPairItr -> first.internalIP,
                                 hostPairItr -> first.externalIP,
                                 hostPairItr -> first.internalPort,
                                 hostPairItr -> first.externalPort,
                                 hostPairItr -> second.numBytes,
                                 hostPairItr -> second.numPackets,
                                 hostPairItr -> second.minPacketSize,
                                 hostPairItr -> second.maxPacketSize,
                                 (void*)&hostPairItr -> second.content,
                                 hostPairItr -> second.startTime,
                                 hostPairItr -> second.endTime,
                                 mySharedState -> ipInformation -> getASN(hostPairItr -> first.externalIP),
                                 mySharedState -> ipInformation -> getCountry(hostPairItr -> first.externalIP),
                                 type)) {
        return -1;
      }
      clock.incrementOperations();
    }
  }
  if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
    return -1;
  }
  clock.stop();
  nonDNSTraffic.clear();
  return clock.operations();
}

bool getP2PStats(PGconn *postgreSQL, const std::string table, uint32_t &ip,
                 uint8_t &type, uint64_t &fanOut, uint64_t &numBytes,
                 uint32_t &startTime, uint32_t &endTime) {
  ostringstream query;
  PGresult *result;
  query << "SELECT COUNT(DISTINCT \"externalIP\"), SUM(\"numBytes\"), "
        << "MIN(\"startTime\"), MAX(\"endTime\") FROM \"NonDNSTraffic\".\""
        << table << "\" WHERE \"internalIP\" = '" << ip << "' AND \"type\" = "
        << (int)type << endl;
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return false;
  }
  fanOut = be64toh(*(uint64_t*)PQgetvalue(result, 0, 0));
  numBytes = be64toh(*(uint64_t*)PQgetvalue(result, 0, 1));
  startTime = ntohl(*(uint32_t*)PQgetvalue(result, 0, 2));
  endTime = ntohl(*(uint32_t*)PQgetvalue(result, 0, 3));
  return true;
}

bool findP2PNodes(PGconn *postgreSQL, const std::string date) {
  ostringstream query;
  PGresult *result;
  uint32_t ip, content[FlowStats::CONTENT_TYPES], startTime, endTime;
  uint64_t fanOut, numBytes;
  uint8_t role, flowType;
  query << "SELECT DISTINCT \"internalIP\", COUNT(DISTINCT \"externalIP\"), "
        << "SUM(\"numBytes\"), SUM(\"content\") FROM \""
        << NON_DNS_TRAFFIC_SCHEMA_NAME << "\".\"" << date
        << "\" GROUP BY \"internalIP\"";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return false;
  }
  for (int row = 0; row < PQntuples(result); ++row) {
    ip = ntohl(*(uint32_t*)PQgetvalue(result, row, 0));
    fanOut = be64toh(*(uint64_t*)PQgetvalue(result, row, 1));
    numBytes = be64toh(*(uint64_t*)PQgetvalue(result, row, 2));
    if (fanOut >= minFanOut || minTraffic >= minTraffic) {
      /*
       * While making a copy of the content sum is not functionally necessary, it
       * will allow a couple of future comparisons to be readable.
       */
      for (size_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
        content[contentType] = ntohl(*(uint32_t*)(char*)(PQgetvalue(result, row, 3) + 24 + (contentType * 8)));
      }
      if ((content[FlowStats::WAV_AUDIO_TYPE] + content[FlowStats::JPEG_IMAGE_TYPE] +
           content[FlowStats::MP3_AUDIO_TYPE] + content[FlowStats::MPEG_VIDEO_TYPE]) >
          (content[FlowStats::PLAINTEXT_TYPE] + content[FlowStats::BMP_IMAGE_TYPE] +
           content[FlowStats::COMPRESSED_TYPE] + content[FlowStats::ENCRYPTED_TYPE])) {
        role = MULTIMEDIA_P2P_NODE;
        flowType = MULTIMEDIA_FLOW;
      }
      else {
        if ((content[FlowStats::COMPRESSED_TYPE] + content[FlowStats::ENCRYPTED_TYPE]) >
            (content[FlowStats::PLAINTEXT_TYPE] + content[FlowStats::BMP_IMAGE_TYPE] +
             content[FlowStats::WAV_AUDIO_TYPE] + content[FlowStats::JPEG_IMAGE_TYPE] +
             content[FlowStats::MP3_AUDIO_TYPE] + content[FlowStats::MPEG_VIDEO_TYPE])) {
          role = ENCRYPTED_P2P_NODE;
          flowType = ENCRYPTED_FLOW;
        }
        else {
          role = UNCLASSIFIED_P2P_NODE;
          flowType = UNCLASSIFIED_FLOW;
        }
      }
      getP2PStats(postgreSQL, date, ip, flowType, fanOut, numBytes, startTime,
                  endTime);
      mySharedState -> roles -> insert(make_pair(ip, Role(role, false, fanOut,
                                                          numBytes, startTime,
                                                          endTime)));
    }
  }
  return true;
}

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &moduleState) {
    mySharedState = &sharedState;


	if (moduleState.conf.get(minFanOut,
							 "min-fan-out",
							 "analysis_nonDNSTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_nonDNSTraffic: missing or invalid min-fan-out"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(minTraffic,
							 "min-traffic",
							 "analysis_nonDNSTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_nonDNSTraffic: missing or invalid min-traffic"
			 << endl;
		return false;
	}
	minTraffic *= 1024 * 1024;


    if (!getWhiteList(mySharedState -> postgreSQL)) {
      return -1;
    }
    for (size_t i = 0; i < whiteList.size(); ++i) {
      cout << ipToText(whiteList[i].first) << ' ' << ipToText(whiteList[i].second) << endl;
    }
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t hour) {
    static TwoWayHostPair hostPair;
    static map <TwoWayHostPair, NonDNSTraffic>::iterator hostPairItr;
    static bool hasResponse;
    if (!inRanges(whiteList, hostPair.externalIP) && isTCP(flowStats)) {
      hostPair = makeTwoWayHostPair(flowStats, *(mySharedState -> localNetworks));
      if (hostPair.externalPort != 53 && hostPair.externalPort != 25) {
        hostPairItr = nonDNSTraffic.find(hostPair);
        if (hostPairItr == nonDNSTraffic.end()) {
          if (!mySharedState -> nameResolution -> hasMapping(hasResponse,
                                                             hostPair.externalIP,
                                                             ims::name::NameResolutionSourceType::DNS)) {
            cerr << "nonDNSTraffic: aggregate(): ERROR: DNS store unavailable." << endl;
            abort();
          }
          if (hasResponse) {
            return;
          }
          hostPairItr = nonDNSTraffic.insert(make_pair(hostPair, NonDNSTraffic())).first;
        }
        hostPairItr -> second.numBytes += flowStats -> numBytes();
        hostPairItr -> second.numPackets += flowStats -> numPackets();
        compareLessThan(hostPairItr -> second.minPacketSize,
                        flowStats -> minPacketSize());
        compareGreaterThan(hostPairItr -> second.maxPacketSize,
                           flowStats -> maxPacketSize());
        compareLessThan(hostPairItr -> second.startTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(hostPairItr -> second.endTime,
                           flowStats -> endTime().seconds());
        accumulateContent(hostPairItr -> second.content,
                          flowStats);
        if (hostPairItr -> second.status == UNKNOWN_STATUS) {
          mySharedState -> connectionInitiators -> aggregate(hostPair, flowStats);
          hostPairItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair,
                                                                                                  flowStats -> sourceIP());
          if (hostPairItr -> second.initiator != TEMPORARILY_UNKNOWN_INITIATOR) {
            if (hostPairItr -> second.initiator == INTERNAL_INITIATOR) {
              hostPairItr -> second.status = BAD_STATUS;
            }
            else {
              nonDNSTraffic.erase(hostPair);
            }
          }
        }
      }
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    int nonDNSTrafficRows;
    nonDNSTrafficRows = commitNonDNSTraffic(pg_conn.connection(), flushSize, date);
    if (nonDNSTrafficRows == -1) {
      return -1;
    }
    if (!findP2PNodes(pg_conn.connection(), date)) {
      return -1;
    }
    return nonDNSTrafficRows;
  }
}
