#include "roles.h"

#include <iostream>
#include <sys/socket.h>
#include <sys/endian.h>
#include <arpa/inet.h>

std::string ntop2(uint32_t numericAddress) {
  in_addr numericAddressStruct;
  char presentationAddress[16];
  numericAddressStruct.s_addr = ntohl(numericAddress);
  inet_ntop(AF_INET, &numericAddressStruct.s_addr, presentationAddress, 16);
  return presentationAddress;
}

Role::Role(uint8_t role, bool dangerous, uint32_t numHosts, uint64_t numBytes,
           uint32_t startTime, uint32_t endTime) {
  _role = role;
  _dangerous = dangerous;
  _numHosts = numHosts;
  _numBytes = numBytes;
  _startTime = startTime;
  _endTime = endTime;
  _excludePort = false;
}

Role::Role(uint8_t role, bool dangerous, uint32_t numHosts, uint64_t numBytes,
           uint32_t startTime, uint32_t endTime, uint16_t excludedPort) {
  _role = role;
  _dangerous = dangerous;
  _numHosts = numHosts;
  _numBytes = numBytes;
  _startTime = startTime;
  _endTime = endTime;
  _excludePort = true;
  _excludedPort = excludedPort;
}

uint32_t getRoleStartTime(PGconn *postgreSQL, const std::string schemaName,
                          const std::string date, const std::string columnName,
                          const uint32_t &ip) {
  std::ostringstream query;
  PGresult *result;
  query << "SELECT \"startTime\" FROM \"" << schemaName << "\".\"" << date
        << '"' << " WHERE \"" << columnName << "\" = '" << ip
        << "' ORDER BY \"startTime\" LIMIT 1";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return 0;
  }
  return ntohl(*(uint32_t*)PQgetvalue(result, 0, 0));
}

uint32_t getRoleEndTime(PGconn *postgreSQL, const std::string schemaName,
                        const std::string date, const std::string columnName,
                        const uint32_t &ip) {
  std::ostringstream query;
  PGresult *result;
  query << "SELECT \"endTime\" FROM \"" << schemaName << "\".\"" << date
        << '"' << " WHERE \"" << columnName << "\" = '" << ip
        << "' ORDER BY \"endTime\" DESC LIMIT 1";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    return 0;
  }
  return ntohl(*(uint32_t*)PQgetvalue(result, 0, 0));
}

std::tr1::unordered_set <uint32_t> getCommChannels(PGconn *postgreSQL,
                                                   const std::string date,
                                                   const uint32_t &ip,
                                                   const uint32_t &startTime) {
  return _getCommChannels(postgreSQL, date, ip, startTime, false, 0);
}

std::tr1::unordered_set <uint32_t> getCommChannels(PGconn *postgreSQL,
                                                   const std::string date,
                                                   const uint32_t &ip,
                                                   const uint32_t &startTime,
                                                   const uint16_t &excludedPort) {
  return _getCommChannels(postgreSQL, date, ip, startTime, true, excludedPort);
}

std::tr1::unordered_set <uint32_t> _getCommChannels(PGconn *postgreSQL,
                                                    const std::string date,
                                                    const uint32_t &ip,
                                                    const uint32_t &startTime,
                                                    const bool &excludePort,
                                                    const uint16_t &excludedPort) {
  std::ostringstream query;
  PGresult *result;
  std::tr1::unordered_set <uint32_t> commChannels;
  if (existsPGTable(postgreSQL, COMM_CHANNELS_SCHEMA_NAME, date) < 1) {
    return commChannels;
  }
  query << "SELECT \"externalIP\" FROM \"CommChannels\".\"" << date << '"'
        << " WHERE \"internalIP\" = '" << ip << "' AND \"startTime\" <= '"
        << startTime << "' ";
  if (excludePort) {
    query << "AND \"externalPort\" != '" << excludedPort << "' ";
  }
  query << "LIMIT 1";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQntuples(result)) {
    commChannels.insert(be32toh(*(uint32_t*)PQgetvalue(result, 0, 0)));
  }
  query.str("");
  query << "SELECT \"externalIP\" FROM \"CommChannels\".\"" << date << '"'
        << " WHERE \"internalIP\" = '" << ip << "' AND \"startTime\" <= '"
        << startTime << "' AND \"endTime\" >= '" << startTime << '\'';
  if (excludedPort) {
    query << " AND \"externalPort\" != '" << excludedPort << '\'';
  }
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQntuples(result)) {
    commChannels.insert(be32toh(*(uint32_t*)PQgetvalue(result, 0, 0)));
  }
  query.str("");
  query << "SELECT \"externalIP\" FROM \"CommChannels\".\"" << date << '"'
        << " WHERE \"internalIP\" = '" << ip << "' AND \"startTime\" >= '"
        << startTime << "' ";
  if (excludedPort) {
    query << "AND \"externalPort\" != '" << excludedPort << "' ";
  }
  query << "LIMIT 1";
  result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
                        NULL, 1);
  if (PQntuples(result)) {
    commChannels.insert(ntohl(*(uint32_t*)PQgetvalue(result, 0, 0)));
  }
  return commChannels;
}
