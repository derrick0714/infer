#include <iostream>
#include <vector>
#include <map>
#include <sys/endian.h>

#include <openssl/bio.h>

#include "modules.hpp"
#include "postgreSQL.h"
#include "getHTTPData.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "MiscHelpers.hpp"

#define INFECTED_IPS_SCHEMA_NAME "InfectedIPs"
#define INFECTED_IPS_TABLE_SCHEMA "\"ip\" uint32 NOT NULL, \
                                   \"sourceNumbers\" uint16[] NOT NULL, \
                                   PRIMARY KEY (\"ip\")"
#define INFECTED_CONTACTS_SCHEMA_NAME "InfectedContacts"
#define INFECTED_CONTACTS_TABLE_SCHEMA "\"protocol\" SMALLINT NOT NULL, \
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
                                        \"weight\" DOUBLE PRECISION NOT NULL, \
                                        \"sourceNumbers\" uint16[] NOT NULL, \
                                        \"asNumber\" uint16 NOT NULL, \
                                        \"countryNumber\" SMALLINT NOT NULL, \
                                        \"content\" uint32[] NOT NULL, \
                                        PRIMARY KEY (\"protocol\", \"internalIP\", \
                                                     \"externalIP\", \"internalPort\", \
                                                     \"externalPort\")"

using namespace std;
using namespace tr1;

class InfectedIP {
  public:
    double weight;
    vector <uint16_t> sourceNumbers;
    InfectedIP();
};

inline InfectedIP::InfectedIP() {
  weight = 1;
}

class InfectedContact {
  public:
    unordered_map <uint32_t, InfectedIP>::iterator infectedIPItr;
    int8_t initiator;
    uint32_t numBytes;
    uint32_t numPackets;
    uint32_t minPacketSize;
    uint32_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    vector <uint32_t> content;
    InfectedContact();
};

inline InfectedContact::InfectedContact() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  numBytes = 0;
  numPackets = 0;
  minPacketSize = numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  startTime = numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

vector <pair <uint32_t, uint32_t> > *localNetworks;
unordered_map <uint32_t, string> *liveIPs;
ConnectionInitiators *connectionInitiators;
IPInformation *ipInformation;

int numInfectedIPs;
TwoWayHostPair hostPair;
unordered_map <uint32_t, InfectedIP> infectedIPs;
unordered_map <uint32_t, uint32_t> imsInfectedIPs;
unordered_map <uint32_t, uint32_t>::iterator imsInfectedIPItr;
map <TwoWayHostPair, InfectedContact> infectedContacts;
unordered_map <uint32_t, InfectedIP>::iterator infectedIPItr;
map <TwoWayHostPair, InfectedContact>::iterator infectedContactItr;

bool getDShieldIPs(uint16_t sourceID) {
  stringstream data;
  string line, numAttacks, octet, ip;
  size_t delimiterPosition, mostAttacks = 0;
  uint32_t numericIP;
  if (!getHTTPData("www.dshield.org", 80, "/ipsascii.html?limit=256", data)) {
    return false;
  }
  while (getline(data, line)) {
    if (line.length() >= 15 && line[3] == '.' && line[7] == '.' &&
        line[11] == '.') {
      delimiterPosition = line.find('\t', 16) + 1;
      numAttacks = line.substr(delimiterPosition,
                               line.find('\t', delimiterPosition) - delimiterPosition);
      if (!mostAttacks) {
        mostAttacks = strtoul(numAttacks.c_str(), NULL, 10);
      }
      line = line.substr(0, 15);
      for (size_t index = 0; index < 15; index += 4) {
        octet = line.substr(index, 3);
        while (octet[0] == '0') {
          octet.erase(octet.begin());
        }                                                                 
        ip += octet;
        if (index < 12) {
          ip += '.';
        }
      }
      numericIP = pton(ip);
      infectedIPs[numericIP].sourceNumbers.push_back(sourceID);
      infectedIPs[numericIP].weight = strtod(numAttacks.c_str(), NULL) / mostAttacks;
      ip.clear();
    }
  }
  return true;
}

bool getCommandAndControlIPs(uint16_t sourceID) {
  stringstream data;
  string line;
  bool inHeader = true;
  size_t delimiterPosition;
  vector <string> IPs;
  if (!getHTTPData("www.emergingthreats.net", 80, "/rules/bleeding-botcc.rules",
                   data)) {
    return false;
  }
  while (getline(data, line)) {
    if (!inHeader && line[0] != '#') {
      delimiterPosition = line.find('[') + 1;
      line = line.substr(delimiterPosition,
                         line.rfind(']') - delimiterPosition);
      if (line[line.length() - 1] == ',') {
        line.erase(line.length() - 1);
      }
      IPs = explodeString(line, ",");
    }
    if (inHeader && line == "\r") {
      inHeader = false;
    }
  }
  for (size_t index = 0; index < IPs.size(); ++index) {
    infectedIPs[pton(IPs[index])].sourceNumbers.push_back(sourceID);
  }
  return true;
}

bool getWaterlooIPs(uint16_t sourceID) {
  stringstream data;
  string line;
  bool inHeader = true;
  if (!getHTTPData("ist.uwaterloo.ca", 80, "/security/trends/Blacklist-28.txt",
                   data)) {
    return false;
  }
  while (getline(data, line)) {
    if (!inHeader) {
      infectedIPs[pton(line)].sourceNumbers.push_back(sourceID);
    }
    if (inHeader && line == "\r") {
      inHeader = false;
    }
  }
  return true;
}

bool getMalwareDomainListIPs(uint16_t sourceID) {
  stringstream data;
  string dataString, ipString;
  size_t index, nextRowPosition = 0, endOfColumnPosition;
  uint32_t ip;
  unordered_map <uint32_t, InfectedIP>::iterator infectedIPItr;
  if (!getHTTPData("www.malwaredomainlist.com", 80,
                   "/mdl.php?search=&colsearch=All&quantity=All", data)) {
    return false;
  }
  dataString = data.str();
  do {
    index = dataString.find("<td>",
                       dataString.find("<td>",
                                       dataString.find("<tr>",
                                                       nextRowPosition)) + 1);
    endOfColumnPosition = dataString.find("</td>", index);
    index += 4;
    while ((isdigit(dataString[index]) || dataString[index] == '.') &&
           index < endOfColumnPosition) {
      ipString += dataString[index++];
    }
    if (ipString.length()) {
      ip = pton(ipString);
      infectedIPItr = infectedIPs.find(ip);
      if (infectedIPItr == infectedIPs.end()) {
        infectedIPItr = infectedIPs.insert(make_pair(ip, InfectedIP())).first;
      }
      if (find(infectedIPItr -> second.sourceNumbers.begin(),
               infectedIPItr -> second.sourceNumbers.end(),
               sourceID) == infectedIPItr -> second.sourceNumbers.end()) {
        infectedIPItr -> second.sourceNumbers.push_back(sourceID);
      }
      ipString.clear();
    }
    nextRowPosition = dataString.find("<tr>", index);
  } while (nextRowPosition != string::npos);
  return true;
}

bool getCompromisedIPs(uint16_t sourceID) {
  stringstream data;
  string line;
  bool inHeader = true;
  size_t delimiterPosition;
  vector <string> networks;
  vector <uint32_t> IPs;
  if (!getHTTPData("www.emergingthreats.net", 80,
                   "/rules/bleeding-compromised.rules", data)) {
    return false;
  }
  while (getline(data, line)) {
    if (!inHeader && line.length() && line[0] != '#') {
      delimiterPosition = line.find('[') + 1;
      line = line.substr(delimiterPosition,
                         line.rfind(']') - delimiterPosition);
      if (line[line.length() - 1] == ',') {
        line.erase(line.length() - 1);
      }
      networks = explodeString(line, ",");
    }
    if (inHeader && line == "\r") {
      inHeader = false;
    }
  }
  for (size_t index = 0; index < networks.size(); ++index) {
    IPs = cidrToIPs(networks[index]);
    for (size_t ip = 0; ip < IPs.size(); ++ip) {
      infectedIPs[IPs[ip]].sourceNumbers.push_back(sourceID);
    }
  }
  return true;
}

bool getIMSIPs(PGconn *postgreSQL, uint16_t sourceID) {
  PGresult *result;
  string query = "SELECT \"ip\", \"lastSeenTime\" FROM \"Maps\".\"infectedIPs\"";
  result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
                        1);
  uint32_t ip;
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return false;
  }
  for (int index = 0; index < PQntuples(result); ++index) {
    ip = be32toh(*(uint32_t*)PQgetvalue(result, index, 0));
    infectedIPs[ip].sourceNumbers.push_back(sourceID);
    if (PQgetlength(result, index, 1) == 0) {
      imsInfectedIPs.insert(make_pair(ip, 0));
    }
    else {
      imsInfectedIPs.insert(make_pair(ip, be32toh(*(uint32_t*)PQgetvalue(result,
                                                                         index,
                                                                         1))));
    }
  }
  return true;
}

bool commitInfectedIPs(PGconn *postgreSQL, size_t &flushSize,
                       const char *date) {
  Clock clock("Inserted", "rows");
  if (!preparePGTable(postgreSQL, INFECTED_IPS_SCHEMA_NAME, date,
                      INFECTED_IPS_TABLE_SCHEMA)) {
    return false;
  }
  PGBulkInserter pgBulkInserter(postgreSQL, INFECTED_IPS_SCHEMA_NAME, date,
                                flushSize, "%ud, %Vh");
  cout << "Updating PostgreSQL with infected IPs" << endl;
  clock.start();
  for (unordered_map <uint32_t, InfectedIP>::iterator infectedIPItr = infectedIPs.begin();
       infectedIPItr != infectedIPs.end(); ++infectedIPItr) {
    if (!pgBulkInserter.insert(NULL, infectedIPItr -> first,
                               (void*)&infectedIPItr -> second.sourceNumbers)) {
      return false;
    }
    clock.incrementOperations();
  }
  if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
    return false;
  }
  clock.stop();
  return true;
}

bool updateLastSeenTimes(PGconn *postgreSQL) {
  ostringstream query;
  Clock clock("Updated", "rows");
  cout << "Updating IMS infected IP last seen times" << endl;
  clock.start();
  for (unordered_map <uint32_t, uint32_t>::iterator infectedIPItr =
       imsInfectedIPs.begin(); infectedIPItr != imsInfectedIPs.end(); ++infectedIPItr) {
    if (infectedIPItr -> second) {
      query << "UPDATE \"Maps\".\"infectedIPs\" SET \"lastSeenTime\" = '"
            << infectedIPItr -> second << "' WHERE \"ip\" = '"
            << infectedIPItr -> first << '\'';
      if (PQresultStatus(PQexec(postgreSQL, query.str().c_str())) != PGRES_COMMAND_OK) {
        return false;
      }
      query.str("");
      clock.incrementOperations();
    }
  }
  clock.stop();
  return true;
}

bool commitInfectedContacts(PGconn *postgreSQL, size_t &flushSize,
                            const char *date) {
  Clock clock("Inserted", "rows");
  if (!preparePGTable(postgreSQL, INFECTED_CONTACTS_SCHEMA_NAME, date,
                      INFECTED_CONTACTS_TABLE_SCHEMA)) {
    return false;
  }
  PGBulkInserter pgBulkInserter(postgreSQL, INFECTED_CONTACTS_SCHEMA_NAME, date,
                                flushSize, "%d, %ud, %ud, %ud, %ud, %ud, " \
                                "%ud, %ud, %ud, %ud, %ud, %ud, %f, %Vuh, " \
                                "%ud, %d, %Vud");
  cout << "Updating PostgreSQL with infected contacts" << endl;
  clock.start();
  for (map <TwoWayHostPair, InfectedContact>::iterator infectedContactItr = infectedContacts.begin();
       infectedContactItr != infectedContacts.end(); ++infectedContactItr) {
    if (infectedContactItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
      infectedContactItr -> second.initiator = PERMANENTLY_UNKNOWN_INITIATOR;
    }                                                                     
    if (!pgBulkInserter.insert(NULL, infectedContactItr -> first.protocol,
                               infectedContactItr -> first.internalIP,
                               infectedContactItr -> first.externalIP,
                               infectedContactItr -> first.internalPort,
                               infectedContactItr -> first.externalPort,
                               infectedContactItr -> second.initiator,
                               infectedContactItr -> second.numBytes,
                               infectedContactItr -> second.numPackets,
                               infectedContactItr -> second.minPacketSize,
                               infectedContactItr -> second.maxPacketSize,
                               infectedContactItr -> second.startTime,
                               infectedContactItr -> second.endTime,
                               infectedContactItr -> second.infectedIPItr -> second.weight,
                               (void*)&infectedContactItr -> second.infectedIPItr -> second.sourceNumbers,
                               ipInformation -> getASN(infectedContactItr -> first.externalIP),
                               ipInformation -> getCountry(infectedContactItr -> first.externalIP),
                               (void*)&infectedContactItr -> second.content)) {
      return false;
    }
    clock.incrementOperations();
  }
  if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
    return false;
  }
  clock.stop();
  return true;
}

extern "C" {
  bool initialize(SharedState &sharedState, ModuleState &) {
    localNetworks = sharedState.localNetworks;
    liveIPs = sharedState.liveIPs;
    connectionInitiators = sharedState.connectionInitiators;
    ipInformation = sharedState.ipInformation;
    if (!getIMSIPs(sharedState.postgreSQL, 1)) {
      //return false;
    }
    if (!getDShieldIPs(2)) {
      //return false;
    }
    if (!getWaterlooIPs(5)) {
      //return false;
    }
    if (!getMalwareDomainListIPs(7)) {
      //return false;
    }
    if (!getCommandAndControlIPs(8)) {
      //return false;
    }
    if (!getCompromisedIPs(9)) {
      //return false;
    }
    numInfectedIPs = commitInfectedIPs(sharedState.postgreSQL,
                                       sharedState.flushSize,
                                       sharedState.date);
    if (numInfectedIPs == -1) {
      return -1;
    }
    return true;
  }

  void aggregate(const FlowStats *flowStats, size_t) {
    hostPair = makeTwoWayHostPair(flowStats, *localNetworks);
    if (liveIPs -> find(hostPair.internalIP) != liveIPs -> end()) {
      infectedIPItr = infectedIPs.find(hostPair.externalIP);
      if (infectedIPItr != infectedIPs.end()) {
        infectedContactItr = infectedContacts.find(hostPair);
        if (infectedContactItr == infectedContacts.end()) {
          infectedContactItr = infectedContacts.insert(make_pair(hostPair, InfectedContact())).first;
          infectedContactItr -> second.infectedIPItr = infectedIPItr;
        }
        if (infectedContactItr -> second.initiator == TEMPORARILY_UNKNOWN_INITIATOR) {
          connectionInitiators -> aggregate(hostPair, flowStats);
          infectedContactItr -> second.initiator = connectionInitiators -> getInitiator(hostPair,
                                                                                        flowStats -> sourceIP());
        }
        infectedContactItr -> second.numBytes += flowStats -> numBytes();
        infectedContactItr -> second.numPackets += flowStats -> numPackets();
        compareLessThan(infectedContactItr -> second.minPacketSize,
                        flowStats -> minPacketSize());
        compareGreaterThan(infectedContactItr -> second.maxPacketSize,
                           flowStats -> maxPacketSize());
        compareLessThan(infectedContactItr -> second.startTime,
                        flowStats -> startTime().seconds());
        compareGreaterThan(infectedContactItr -> second.endTime,
                           flowStats -> endTime().seconds());
        accumulateContent(infectedContactItr -> second.content,
                          flowStats);
        imsInfectedIPItr = imsInfectedIPs.find(hostPair.externalIP);
        if (imsInfectedIPItr != imsInfectedIPs.end()) {
          compareGreaterThan(imsInfectedIPItr -> second,
                             flowStats -> endTime().seconds());
        }
      }
    }
  }

  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
    int numInfectedContacts;
    if (!updateLastSeenTimes(pg_conn.connection())) {
      return -1;
    }
    numInfectedContacts = commitInfectedContacts(pg_conn.connection(), flushSize, date);
    if (numInfectedContacts == -1) {
      return -1;
    }
    return (numInfectedIPs + numInfectedContacts);
  }
}
