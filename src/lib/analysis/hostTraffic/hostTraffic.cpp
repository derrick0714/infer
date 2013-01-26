#include <iostream>
#include <sstream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <sys/endian.h>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "MiscHelpers.hpp"

#include "hostPairTraffic.hpp"
#include "hostTraffic.hpp"
#include "portIP.hpp"

#define HOST_TRAFFIC_SCHEMA_NAME "HostTraffic"
#define HOST_TRAFFIC_TABLE_SCHEMA "\"ip\" uint32 NOT NULL, \
                                   \"inboundFlows\" uint32 NOT NULL, \
                                   \"outboundFlows\" uint32 NOT NULL, \
                                   \"inboundPackets\" uint32 NOT NULL, \
                                   \"outboundPackets\" uint32 NOT NULL, \
                                   \"inboundBytes\" uint64 NOT NULL, \
                                   \"outboundBytes\" uint64 NOT NULL, \
                                   \"inboundPortTraffic\" uint64[] NOT NULL, \
                                   \"inboundPortFirstActivityTimes\" uint32[] NOT NULL, \
                                   \"inboundPortLastActivityTimes\" uint32[] NOT NULL, \
                                   \"inboundPortIPs\" uint32[] NOT NULL, \
                                   \"outboundPortTraffic\" uint64[] NOT NULL, \
                                   \"outboundPortFirstActivityTimes\" uint32[] NOT NULL, \
                                   \"outboundPortLastActivityTimes\" uint32[] NOT NULL, \
                                   \"outboundPortIPs\" uint32[] NOT NULL, \
                                   \"inboundContent\" uint32[] NOT NULL, \
                                   \"outboundContent\" uint32[] NOT NULL, \
                                   \"firstInboundTime\" uint32 NOT NULL, \
                                   \"lastInboundTime\" uint32 NOT NULL, \
                                   \"firstOutboundTime\" uint32 NOT NULL, \
                                   \"lastOutboundTime\" uint32 NOT NULL, \
                                   \"roles\" uint16 NOT NULL, \
                                   PRIMARY KEY (\"ip\")"

#define PORT_IPS_SCHEMA_NAME "PortIPs"
#define PORT_IPS_TABLE_SCHEMA "\"internalIP\" uint32 NOT NULL, \
                               \"externalIP\" uint32 NOT NULL, \
                               \"port\" uint16 NOT NULL, \
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
							   \"relevance\" DOUBLE PRECISION, \
                               PRIMARY KEY (\"internalIP\", \"externalIP\", \
                                            \"port\", \"initiator\")"

using namespace std;
using namespace tr1;

unordered_map <uint16_t, size_t> interestingPorts;

SharedState *mySharedState;
size_t darkSpaceThreshold;

float trafficDifference, proxyDifference;
size_t proxyThreshold;
map <TwoWayHostPair, HostPairTraffic> hostPairTraffic;
unordered_set <uint32_t> existingInterestingIPs;
unordered_map <uint32_t, HostTraffic> hostTraffic;
map <PortIPHostPair, PortIP> portIPs;
int8_t initiator;

void loadInterestingPorts(const string &_string) {
  size_t startPosition = 0, endPosition, index = 0;
  do {
    endPosition = _string.find(' ', startPosition);
    interestingPorts.insert(make_pair(strtoul(_string.substr(startPosition,
                                                             endPosition - startPosition).c_str(),
                                              NULL, 10), index++));
    startPosition = endPosition + 1;
  } while (endPosition != string::npos);
}

/*
 * Assigns the dark space bot role to any internal host that has sent traffic to at
 * least as many non-live hosts as configured by the darkSpaceThreshold variable.
 */
bool findDarkSpaceBots(PGconn *postgreSQL, const char *date) {
  ostringstream query;
  PGresult *result;
  uint32_t ip, numHosts, startTime;
  if (existsPGTable(postgreSQL, DARK_SPACE_SOURCES_SCHEMA_NAME, date) == 1 &&
      existsPGTable(postgreSQL, COMM_CHANNELS_SCHEMA_NAME, date) == 1) {
    query << "SELECT \"sourceIP\", COUNT(DISTINCT \"destinationIP\"), "
          << "SUM(\"numBytes\") FROM \"" << DARK_SPACE_SOURCES_SCHEMA_NAME
          << "\".\"" << date
          << "\" GROUP BY \"sourceIP\" ORDER BY COUNT(DISTINCT \"destinationIP\")"
          << " DESC";
    result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                          NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
      return false;
    }
    for (int row = 0; row < PQntuples(result); ++row) {
      ip = ntohl(*(uint32_t*)PQgetvalue(result, row, 0));
      numHosts = be64toh(*(uint64_t*)PQgetvalue(result, row, 1));
      if (isInternal(ip, *(mySharedState -> localNetworks)) && numHosts >= darkSpaceThreshold) {
        startTime = getRoleStartTime(postgreSQL, DARK_SPACE_SOURCES_SCHEMA_NAME,
                                     date, "sourceIP", ip);
        mySharedState -> roles -> insert(make_pair(ip,
                                                   Role(DARK_SPACE_BOT, true, numHosts,
                                                   be32toh(*(uint32_t*)PQgetvalue(result, row,
                                                                                  2)),
                                                   startTime,
                                                   getRoleEndTime(postgreSQL,
                                                                  DARK_SPACE_SOURCES_SCHEMA_NAME,
                                                                  date, "sourceIP",
                                                                  ip))));
      }
    }
  }
  return true;
}

bool findSpamBots() {
  unordered_set <uint32_t> uniqueCounts;
  vector <uint32_t> counts;
  uint32_t count, median;
  uint64_t numBytes;
  for (unordered_map <uint32_t, HostTraffic>::iterator hostTrafficItr = hostTraffic.begin();
       hostTrafficItr != hostTraffic.end(); ++hostTrafficItr) {
    count = hostTrafficItr -> second.outboundPortIPs[0];
    if (count > 0 && uniqueCounts.find(count) == uniqueCounts.end()) {
      uniqueCounts.insert(count);
      counts.push_back(count);
    }
  }
  if (counts.size()) {
    sort(counts.begin(), counts.end());
    if (counts.size() % 2 != 0) {
      median = counts[counts.size() / 2];
    }
    else {
      median = (counts[(counts.size() / 2) - 1] + counts[counts.size() / 2]) / 2;
    }
    for (unordered_map <uint32_t, HostTraffic>::iterator hostTrafficItr = hostTraffic.begin();
         hostTrafficItr != hostTraffic.end(); ++hostTrafficItr) {
      if (hostTrafficItr -> second.outboundPortIPs[0] >= median &&
          !hostTrafficItr -> second.inboundPortIPs[0] &&
          !hostTrafficItr -> second.inboundPortIPs[2] &&
          !hostTrafficItr -> second.inboundPortIPs[3] &&
          !hostTrafficItr -> second.inboundPortIPs[5] &&
          !hostTrafficItr -> second.inboundPortIPs[6] &&
          !hostTrafficItr -> second.inboundPortIPs[7]) {
        numBytes = 0;
        numBytes += hostTrafficItr -> second.outboundPortTraffic[0];
        mySharedState -> roles -> insert(make_pair(hostTrafficItr -> first, Role(SPAM_BOT, true,
                                                                                 hostTrafficItr -> second.outboundPortIPs[0],
                                                                                 numBytes,
                                                                                 hostTrafficItr -> second.outboundPortFirstActivityTimes[0],
                                                                                 hostTrafficItr -> second.outboundPortLastActivityTimes[0],
                                                                                 25)));
      }
    }
  }
  return true;
}

bool getBruteForcerRoles(PGconn *postgreSQL, const char *date) {
  ostringstream query;
  PGresult *result;
  uint16_t port, role = 0;
  uint32_t ip, startTime, numBytes;
  bool rolePort;
  if (existsPGTable(postgreSQL, BRUTE_FORCERS_SCHEMA_NAME, date) == 1) {
    query << "SELECT \"sourceIP\", \"destinationPort\", "
          << "COUNT(DISTINCT \"destinationIP\"), SUM(\"numBytes\") FROM \""
          << BRUTE_FORCERS_SCHEMA_NAME << "\".\"" << date
          << "\" GROUP BY \"destinationPort\", \"sourceIP\"";
    result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                          NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
      return false;
    }
    for (int index = 0; index < PQntuples(result); ++index) {
      ip = ntohl(*(uint32_t*)PQgetvalue(result, index, 0));
      if (isInternal(ip, *(mySharedState -> localNetworks))) {
        startTime = getRoleStartTime(postgreSQL, BRUTE_FORCERS_SCHEMA_NAME,
                                     date, "sourceIP", ip);
        port = ntohs(*(uint16_t*)PQgetvalue(result, index, 1));
        numBytes = ntohl(*(uint32_t*)PQgetvalue(result, index, 3));
        rolePort = true;
        switch (port) {
          /* FTP. */
          case 21:
            role = FTP_BRUTE_FORCER;
            break;
          /* SSH. */
          case 22:
            role = SSH_BRUTE_FORCER;
            break;
          /* Telnet. */
          case 23:
            role = TELNET_BRUTE_FORCER;
            break;
          /* Microsoft SQL. */
          case 1433:
            role = MICROSOFT_SQL_BRUTE_FORCER;
            break;
          /* Oracle SQL. */
          case 1521:
            role = ORACLE_SQL_BRUTE_FORCER;
            break;
          /* MySQL. */
          case 3306:
            role = MYSQL_BRUTE_FORCER;
            break;
          /* PostgreSQL. */
          case 5432:
            role = POSTGRESQL_BRUTE_FORCER;
            break;
          default:
            rolePort = false;
            break;
        }
        if (rolePort) {
          switch (role) {
            case FTP_BRUTE_FORCER:
              mySharedState -> roles -> insert(make_pair(ip, Role(role, false,
                                                                  be64toh(*(uint64_t*)PQgetvalue(result,
                                                                                                 index, 2)),
                                                                  numBytes, startTime,
                                                                  getRoleEndTime(postgreSQL,
                                                                                 BRUTE_FORCERS_SCHEMA_NAME,
                                                                                 date, "sourceIP", ip),
                                                                  port)));
              break;
            case SSH_BRUTE_FORCER:
            case TELNET_BRUTE_FORCER:
            case MICROSOFT_SQL_BRUTE_FORCER:
            case ORACLE_SQL_BRUTE_FORCER:
            case MYSQL_BRUTE_FORCER:
            case POSTGRESQL_BRUTE_FORCER:
              mySharedState -> roles -> insert(make_pair(ip, Role(role, true,
                                                                  be64toh(*(uint64_t*)PQgetvalue(result,
                                                                                                 index,
                                                                                                 2)),
                                                                  numBytes, startTime,
                                                                  getRoleEndTime(postgreSQL,
                                                                                 BRUTE_FORCERS_SCHEMA_NAME,
                                                                                 date, "sourceIP", ip),
                                                                  port)));
              break;
          }
        }
      }
    }
  }
  return true;
}

bool getBruteForcedRoles(PGconn *postgreSQL, const char *date) {
  ostringstream query;
  PGresult *result;
  uint16_t role = 0;
  uint32_t ip;
  bool rolePort;
  if (existsPGTable(postgreSQL, BRUTE_FORCERS_SCHEMA_NAME, date) == 1) {
    query << "SELECT \"destinationIP\", \"destinationPort\", "
          << "COUNT(DISTINCT \"sourceIP\"), SUM(\"numBytes\") FROM \""
          << BRUTE_FORCERS_SCHEMA_NAME << "\".\"" << date
          << "\" GROUP BY \"destinationIP\", \"destinationPort\"";
    result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                          NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
      return false;
    }
    for (int index = 0; index < PQntuples(result); ++index) {
      ip = ntohl(*(uint32_t*)PQgetvalue(result, index, 0));
      rolePort = true;
      if (isInternal(ip, *(mySharedState -> localNetworks))) {
        switch (ntohs(*(uint16_t*)PQgetvalue(result, index, 1))) {
          /* FTP. */
          case 21:
            role = FTP_BRUTE_FORCED;
            break;
          /* SSH. */
          case 22:
            role = SSH_BRUTE_FORCED;
            break;
          /* Telnet. */
          case 23:
            role = TELNET_BRUTE_FORCED;
            break;
          /* Microsoft SQL. */
          case 1433:
            role = MICROSOFT_SQL_BRUTE_FORCED;
            break;
          /* Oracle SQL. */
          case 1521:
            role = ORACLE_SQL_BRUTE_FORCED;
            break;
          /* MySQL. */
          case 3306:
            role = MYSQL_BRUTE_FORCED;
            break;
          /* PostgreSQL. */
          case 5432:
            role = POSTGRESQL_BRUTE_FORCED;
            break;
          default:
            rolePort = false;
            break;
        }
        if (rolePort) {
          mySharedState -> roles -> insert(make_pair(ip, Role(role, false,
                                                              be64toh(*(uint64_t*)PQgetvalue(result,
                                                                                             index,
                                                                                             2)),
                                                              ntohl(*(uint32_t*)PQgetvalue(result,
                                                                                           index, 3)),
                                                              getRoleStartTime(postgreSQL,
                                                                               BRUTE_FORCERS_SCHEMA_NAME,
                                                                               date, "destinationIP", ip),
                                                              getRoleEndTime(postgreSQL,
                                                                             BRUTE_FORCERS_SCHEMA_NAME,
                                                                             date, "destinationIP", ip))));
        }
      }
    }
  }
  return true;
}

bool findScanBots(PGconn *postgreSQL, const char *date) {
  ostringstream query;
  PGresult *result;
  uint32_t ip;
  if (existsPGTable(postgreSQL, SCANNERS_SCHEMA_NAME, date) == 1 &&
      existsPGTable(postgreSQL, COMM_CHANNELS_SCHEMA_NAME, date) == 1) {
    query << "SELECT DISTINCT \"sourceIP\", COUNT(DISTINCT \"destinationIP\"), "
          << "SUM(\"numBytes\") FROM \"" << SCANNERS_SCHEMA_NAME << "\".\"" << date
          << "\" GROUP BY \"sourceIP\"";
    result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                          NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
      return false;
    }
    for (int row = 0; row < PQntuples(result); ++row) {
      ip = ntohl(*(uint32_t*)PQgetvalue(result, row, 0));
      if (isInternal(ip, *(mySharedState -> localNetworks))) {
        mySharedState -> roles -> insert(make_pair(ip, Role(SCAN_BOT, true,
                                                            be64toh(*(uint64_t*)PQgetvalue(result,
                                                                                           row,
                                                                                           1)),
                                                            ntohl(*(uint32_t*)PQgetvalue(result,
                                                                                         row,
                                                                                         2)),
                                                            getRoleStartTime(postgreSQL,
                                                                             SCANNERS_SCHEMA_NAME,
                                                                             date, "sourceIP", ip),
                                                            getRoleEndTime(postgreSQL,
                                                                           SCANNERS_SCHEMA_NAME,
                                                                           date, "sourceIP", ip))));
                                  
      }
    }
  }
  return true;
}

bool updateInterestingPortIndex(PGconn *postgreSQL, const char *date) {
  vector <uint16_t> _interestingPorts;
  for (unordered_map <uint16_t, size_t>::iterator interestingPortItr = interestingPorts.begin();
       interestingPortItr != interestingPorts.end(); ++interestingPortItr) {
    _interestingPorts.push_back(interestingPortItr -> first);
  }
  sort(_interestingPorts.begin(), _interestingPorts.end());
  if (!deletePGRows(postgreSQL, "Indexes", "interestingPorts", "date", date)) {
    return false;
  }
  if (!insertPGRow(postgreSQL, "Indexes", "interestingPorts", "%s, %Vuh", date,
                   (void*)&_interestingPorts)) {
    return false;
  }
  return true;
}

void processHostPairTraffic() {
  map <TwoWayHostPair, HostPairTraffic>::iterator hostPairTrafficItr = hostPairTraffic.begin();
  unordered_map <uint32_t, HostTraffic>::iterator hostTrafficItr = hostTraffic.begin();
  unordered_map <uint16_t, size_t>::iterator interestingPortItr;
  map <PortIPHostPair, PortIP>::iterator portIPItr;
  PortIPHostPair portIPHostPair;
  bool internalDestination = false;
  while (!hostPairTraffic.empty()) {
    switch (hostPairTrafficItr -> second.initiator) {
      case INTERNAL_INITIATOR:
        interestingPortItr = interestingPorts.find(hostPairTrafficItr -> first.externalPort);
        if (interestingPortItr == interestingPorts.end()) {
          break;
        }
        hostTrafficItr = hostTraffic.find(hostPairTrafficItr -> first.internalIP);
        if (hostTrafficItr == hostTraffic.end()) {
          hostTrafficItr = hostTraffic.insert(make_pair(hostPairTrafficItr -> first.internalIP,
                                                          HostTraffic(interestingPorts.size()))).first;
        }
        hostTrafficItr -> second.outboundPortTraffic[interestingPortItr -> second] += hostPairTrafficItr -> second.numBytes;
        compareLessThan(hostTrafficItr -> second.outboundPortFirstActivityTimes[interestingPortItr -> second],
                        hostPairTrafficItr -> second.startTime);
        compareGreaterThan(hostTrafficItr -> second.outboundPortLastActivityTimes[interestingPortItr -> second],
                           hostPairTrafficItr -> second.endTime);
        portIPHostPair = PortIPHostPair(hostPairTrafficItr -> first.internalIP,
                                        hostPairTrafficItr -> first.externalIP,
                                        hostPairTrafficItr -> first.externalPort,
                                        hostPairTrafficItr -> second.initiator);
        portIPItr = portIPs.find(portIPHostPair);
        if (portIPItr == portIPs.end()) {
          portIPItr = portIPs.insert(make_pair(portIPHostPair, PortIP())).first;
        }
        portIPItr -> second.numBytes += hostPairTrafficItr -> second.numBytes;
        accumulateContent(portIPItr -> second.content,
                          hostPairTrafficItr -> second.content);
        portIPItr -> second.numPackets += hostPairTrafficItr -> second.numPackets;
        compareLessThan(portIPItr -> second.minPacketSize,
                        hostPairTrafficItr -> second.minPacketSize);
        compareGreaterThan(portIPItr -> second.maxPacketSize,
                           hostPairTrafficItr -> second.maxPacketSize);
        compareLessThan(portIPItr -> second.startTime,
                        hostPairTrafficItr -> second.startTime);
        compareGreaterThan(portIPItr -> second.endTime,
                           hostPairTrafficItr -> second.endTime);
        if (isInternal(hostPairTrafficItr -> first.externalIP,
            *(mySharedState -> localNetworks))) {
          internalDestination = true;
        }
        else {
          break;
        }
      case EXTERNAL_INITIATOR:
        if (!internalDestination) {
          interestingPortItr = interestingPorts.find(hostPairTrafficItr -> first.internalPort);
          if (interestingPortItr == interestingPorts.end()) {
            break;
          }
          hostTrafficItr = hostTraffic.find(hostPairTrafficItr -> first.internalIP);
          if (hostTrafficItr == hostTraffic.end()) {
            hostTrafficItr = hostTraffic.insert(make_pair(hostPairTrafficItr -> first.internalIP,
                                                          HostTraffic(interestingPorts.size()))).first;
          }
          portIPHostPair = PortIPHostPair(hostPairTrafficItr -> first.internalIP,
                                          hostPairTrafficItr -> first.externalIP,
                                          hostPairTrafficItr -> first.internalPort,
                                          hostPairTrafficItr -> second.initiator);
        }
        else {
          hostTrafficItr = hostTraffic.find(hostPairTrafficItr -> first.externalIP);
          if (hostTrafficItr == hostTraffic.end()) {
            hostTrafficItr = hostTraffic.insert(make_pair(hostPairTrafficItr -> first.externalIP,
                                                          HostTraffic(interestingPorts.size()))).first;
          }
          portIPHostPair = PortIPHostPair(hostPairTrafficItr -> first.externalIP,
                                          hostPairTrafficItr -> first.internalIP,
                                          hostPairTrafficItr -> first.externalPort,
                                          EXTERNAL_INITIATOR);
          internalDestination = false;
        }
        hostTrafficItr -> second.inboundPortTraffic[interestingPortItr -> second] += hostPairTrafficItr -> second.numBytes;
        compareLessThan(hostTrafficItr -> second.inboundPortFirstActivityTimes[interestingPortItr -> second],
                        hostPairTrafficItr -> second.startTime);
        compareGreaterThan(hostTrafficItr -> second.inboundPortLastActivityTimes[interestingPortItr -> second],
                           hostPairTrafficItr -> second.endTime);
        portIPItr = portIPs.find(portIPHostPair);
        if (portIPItr == portIPs.end()) {
          portIPItr = portIPs.insert(make_pair(portIPHostPair, PortIP())).first;
        }
        portIPItr -> second.numBytes += hostPairTrafficItr -> second.numBytes;
        accumulateContent(portIPItr -> second.content,
                          hostPairTrafficItr -> second.content);
        portIPItr -> second.numPackets += hostPairTrafficItr -> second.numPackets;
        compareLessThan(portIPItr -> second.minPacketSize,
                        hostPairTrafficItr -> second.minPacketSize);
        compareGreaterThan(portIPItr -> second.maxPacketSize,
                           hostPairTrafficItr -> second.maxPacketSize);
        compareLessThan(portIPItr -> second.startTime,
                        hostPairTrafficItr -> second.startTime);
        compareGreaterThan(portIPItr -> second.endTime,
                           hostPairTrafficItr -> second.endTime);
        break;
    }
    hostPairTraffic.erase(hostPairTraffic.begin());
    hostPairTrafficItr = hostPairTraffic.begin();
  }
  for (portIPItr = portIPs.begin(); portIPItr != portIPs.end(); ++portIPItr) {
    hostTrafficItr = hostTraffic.find(portIPItr -> first.internalIP);
    interestingPortItr = interestingPorts.find(portIPItr -> first.port);
    if (portIPItr -> first.initiator == INTERNAL_INITIATOR) {
      ++(hostTrafficItr -> second.outboundPortIPs[interestingPortItr -> second]);
    }
    else {
      ++(hostTrafficItr -> second.inboundPortIPs[interestingPortItr -> second]);
    }
  }
}

int commitHostTraffic(PGconn *postgreSQL, size_t &flushSize, const char *date) {
  Clock clock("Inserted", "rows");
  PGBulkInserter pgBulkInserter(postgreSQL, HOST_TRAFFIC_SCHEMA_NAME, date,
                                flushSize, "%ud, %ud, %ud, %ud, %ud, " \
                                "%ul, %ul, %Vul, %Vud, %Vud, %Vud, %Vul, " \
                                "%Vud, %Vud, %Vud, %Vud, %Vud, %ud, %ud, " \
                                "%ud, %ud, %ud");
  if (!preparePGTable(postgreSQL, HOST_TRAFFIC_SCHEMA_NAME, date,
                      HOST_TRAFFIC_TABLE_SCHEMA)) {
    return -1;
  }
  cout << "Updating PostgreSQL database with host traffic" << endl;
  clock.start();
  for (unordered_map <uint32_t, HostTraffic>::iterator internalIPItr = hostTraffic.begin();
       internalIPItr != hostTraffic.end(); ++internalIPItr) {
    if (!pgBulkInserter.insert(NULL, internalIPItr -> first,
                               internalIPItr -> second.inboundFlows,
                               internalIPItr -> second.outboundFlows,
                               internalIPItr -> second.inboundPackets,
                               internalIPItr -> second.outboundPackets,
                               internalIPItr -> second.inboundBytes,
                               internalIPItr -> second.outboundBytes,
                               (void*)&(internalIPItr -> second.inboundPortTraffic),
                               (void*)&(internalIPItr -> second.inboundPortFirstActivityTimes),
                               (void*)&(internalIPItr -> second.inboundPortLastActivityTimes),
                               (void*)&(internalIPItr -> second.inboundPortIPs),
                               (void*)&(internalIPItr -> second.outboundPortTraffic),
                               (void*)&(internalIPItr -> second.outboundPortFirstActivityTimes),
                               (void*)&(internalIPItr -> second.outboundPortLastActivityTimes),
                               (void*)&(internalIPItr -> second.outboundPortIPs),
                               (void*)&(internalIPItr -> second.inboundContent),
                               (void*)&(internalIPItr -> second.outboundContent),
                               internalIPItr -> second.firstInboundTime,
                               internalIPItr -> second.lastInboundTime,
                               internalIPItr -> second.firstOutboundTime,
                               internalIPItr -> second.lastOutboundTime,
                               internalIPItr -> second.roles)) {
      return -1;
    }
    clock.incrementOperations();
  }
  if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
    return -1;
  }
  clock.stop();
  hostTraffic.clear();
  return clock.operations();
}

int commitPortIPs(PGconn *postgreSQL, size_t &flushSize, const char *date) {
  Clock clock("Inserted", "rows");
  PGBulkInserter pgBulkInserter(postgreSQL, PORT_IPS_SCHEMA_NAME, date,
                                flushSize, "%ud, %ud, %ud, %ud, %ud, %ud, " \
                                "%ud, %ud, %ud, %ud, %ud, %d, %Vud, %f");
  if (!preparePGTable(postgreSQL, PORT_IPS_SCHEMA_NAME, date,
                      PORT_IPS_TABLE_SCHEMA)) {
    return -1;
  }
  cout << "Updating PostgreSQL database with port IPs" << endl;
  clock.start();
  for (map <PortIPHostPair, PortIP>::iterator portIPItr = portIPs.begin();
       portIPItr != portIPs.end(); ++portIPItr) {
    if (!pgBulkInserter.insert(NULL, portIPItr -> first.internalIP,
                               portIPItr -> first.externalIP,
                               portIPItr -> first.port,
                               portIPItr -> first.initiator,
                               portIPItr -> second.numBytes,
                               portIPItr -> second.numPackets,
                               portIPItr -> second.minPacketSize,
                               portIPItr -> second.maxPacketSize,
                               portIPItr -> second.startTime,
                               portIPItr -> second.endTime,
                               mySharedState -> ipInformation -> getASN(portIPItr -> first.externalIP),
                               mySharedState ->ipInformation -> getCountry(portIPItr -> first.externalIP),
                               (void*)&(portIPItr -> second.content),
							   0.0)) {
      return -1;
    }
    clock.incrementOperations();
  }
  if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
    return -1;
  }
  clock.stop();
  portIPs.clear();
  return clock.operations();
}

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &moduleState) {
    mySharedState = &sharedState;
	
	string interesting_ports;
	if (moduleState.conf.get(interesting_ports,
							 "interesting-ports",
							 "analysis_hostTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_hostTraffic: missing or invalid interesting-ports"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(trafficDifference,
							 "traffic-difference",
							 "analysis_hostTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_hostTraffic: missing or invalid traffic-difference"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(proxyThreshold,
							 "proxy-threshold",
							 "analysis_hostTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_hostTraffic: missing or invalid proxy-threshold"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(proxyDifference,
							 "proxy-difference",
							 "analysis_hostTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_hostTraffic: missing or invalid proxy-difference"
			 << endl;
		return false;
	}

	if (moduleState.conf.get(darkSpaceThreshold,
							 "darkspace-threshold",
							 "analysis_hostTraffic")
			!= configuration::OK)
	{
		cerr << "analysis_hostTraffic: missing or invalid darkspace-threshold"
			 << endl;
		return false;
	}

    loadInterestingPorts(interesting_ports);

    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    static TwoWayHostPair hostPair, tempHostPair;
    static unordered_map <uint32_t, HostTraffic>::iterator hostTrafficItr;
    static map <TwoWayHostPair, HostPairTraffic>::iterator hostPairTrafficItr;
    static unordered_map <uint16_t, size_t>::iterator interestingPortItr;
    static bool internalSource;
    if (isTCP(flowStats)) {
      hostPair = makeTwoWayHostPair(flowStats, *(mySharedState -> localNetworks));
      mySharedState -> connectionInitiators -> aggregate(hostPair, flowStats);
      internalSource = false;
      /* Check whether the source IP is internal and live. */
      if (mySharedState -> liveIPs -> find(flowStats -> sourceIP()) != mySharedState -> liveIPs -> end()) {
        internalSource = true;
        hostTrafficItr = hostTraffic.find(flowStats -> sourceIP());
        interestingPortItr = interestingPorts.find(flowStats -> destinationPort());
        if (hostTrafficItr == hostTraffic.end()) {
          hostTrafficItr = hostTraffic.insert(make_pair(flowStats -> sourceIP(),
                                                        HostTraffic(interestingPorts.size()))).first;
        }
        compareLessThan(hostTrafficItr -> second.firstActivityTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(hostTrafficItr -> second.lastActivityTime,
                           flowStats -> endTime().seconds());
        ++(hostTrafficItr -> second.outboundFlows);
        hostTrafficItr -> second.outboundPackets += flowStats -> numPackets();
        hostTrafficItr -> second.outboundBytes += flowStats -> numBytes();
        accumulateContent(hostTrafficItr -> second.outboundContent,
                          flowStats);
        compareLessThan(hostTrafficItr -> second.firstOutboundTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(hostTrafficItr -> second.lastOutboundTime,
                           flowStats -> endTime().seconds());
        /* Check whether the destination port is interesting. */
        if (interestingPortItr != interestingPorts.end()) {
          hostPairTrafficItr = hostPairTraffic.find(hostPair);
          if (hostPairTrafficItr == hostPairTraffic.end()) {
            hostPairTrafficItr = hostPairTraffic.insert(make_pair(hostPair, HostPairTraffic())).first;
          }
          if (hostPairTrafficItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
            hostPairTrafficItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair,
                                                                                                           flowStats -> sourceIP());
          }
          compareLessThan(hostPairTrafficItr -> second.startTime,
                          flowStats -> startTime().seconds());
          compareGreaterThan(hostPairTrafficItr -> second.endTime,
                             flowStats -> endTime().seconds());
          hostPairTrafficItr -> second.numPackets += flowStats -> numPackets();
          hostPairTrafficItr -> second.numBytes += flowStats -> numBytes();
          compareLessThan(hostPairTrafficItr -> second.minPacketSize,
                          flowStats -> minPacketSize());
          compareGreaterThan(hostPairTrafficItr -> second.maxPacketSize,
                             flowStats -> maxPacketSize());
          accumulateContent(hostPairTrafficItr -> second.content,
                            flowStats);
        }
      }
      hostPairTrafficItr = hostPairTraffic.find(hostPair);
      if (hostPairTrafficItr != hostPairTraffic.end()) {
        hostPairTrafficItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair, flowStats -> sourceIP());
      }
      /* Check whether the destination IP is internal and live. */
      if (mySharedState -> liveIPs -> find(flowStats -> destinationIP()) != mySharedState -> liveIPs -> end()) {
        hostTrafficItr = hostTraffic.find(flowStats -> destinationIP());
        interestingPortItr = interestingPorts.find(flowStats -> destinationPort());
        if (hostTrafficItr == hostTraffic.end()) {
          hostTrafficItr = hostTraffic.insert(make_pair(flowStats -> destinationIP(),
                                                        HostTraffic(interestingPorts.size()))).first;
        }
        ++(hostTrafficItr -> second.inboundFlows);
        hostTrafficItr -> second.inboundPackets += flowStats -> numPackets();
        hostTrafficItr -> second.inboundBytes += flowStats -> numBytes();
        accumulateContent(hostTrafficItr -> second.inboundContent,
                          flowStats);
        compareLessThan(hostTrafficItr -> second.firstInboundTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(hostTrafficItr -> second.lastInboundTime,
                           flowStats -> endTime().seconds());
        /* Check whether the destination port is interesting. */
        if (interestingPortItr != interestingPorts.end()) {
          if (internalSource) {
            tempHostPair = hostPair;
            hostPair.internalIP = tempHostPair.externalIP;
            hostPair.externalIP = tempHostPair.internalIP;
            hostPair.internalPort = tempHostPair.externalPort;
            hostPair.externalPort = tempHostPair.internalPort;
            mySharedState -> connectionInitiators -> aggregate(hostPair, flowStats);
            hostPairTrafficItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair,
                                                                                                           flowStats -> sourceIP());
          }
          hostPairTrafficItr = hostPairTraffic.find(hostPair);
          if (hostPairTrafficItr == hostPairTraffic.end()) {
            hostPairTrafficItr = hostPairTraffic.insert(make_pair(hostPair, HostPairTraffic())).first;
          }
          compareLessThan(hostPairTrafficItr -> second.startTime,
                          flowStats -> startTime().seconds());
          compareGreaterThan(hostPairTrafficItr -> second.endTime,
                             flowStats -> endTime().seconds());
          hostPairTrafficItr -> second.numPackets += flowStats -> numPackets();
          hostPairTrafficItr -> second.numBytes += flowStats -> numBytes();
          compareLessThan(hostPairTrafficItr -> second.minPacketSize,
                          flowStats -> minPacketSize());
          compareGreaterThan(hostPairTrafficItr -> second.maxPacketSize,
                             flowStats -> maxPacketSize());
          accumulateContent(hostPairTrafficItr -> second.content,
                            flowStats);
        }
      }
      hostPairTrafficItr = hostPairTraffic.find(hostPair);
      if (hostPairTrafficItr != hostPairTraffic.end()) {
        hostPairTrafficItr -> second.initiator = mySharedState -> connectionInitiators -> getInitiator(hostPair, flowStats -> sourceIP());
      }
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    int numHostTrafficRows, numPortIPs;
    processHostPairTraffic(); 
    if (!findDarkSpaceBots(pg_conn.connection(), date)) {
      return -1;
    }
    if (!findSpamBots()) {
      return -1;
    }
    if (!findScanBots(pg_conn.connection(), date)) {
      return -1;
    }
    if (!getBruteForcerRoles(pg_conn.connection(), date)) {
      return -1;
    }
    if (!getBruteForcedRoles(pg_conn.connection(), date)) {
      return -1;
    }
    if (!updateInterestingPortIndex(pg_conn.connection(), date)) {
      return -1;
    }
    numHostTrafficRows = commitHostTraffic(pg_conn.connection(), flushSize, date);
    if (numHostTrafficRows == -1) {
      return -1;
    }
    numPortIPs = commitPortIPs(pg_conn.connection(), flushSize, date);
    if (numPortIPs == -1) {
      return -1;
    }
    return (numHostTrafficRows + numPortIPs);
  }
}
