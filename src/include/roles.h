#ifndef ROLES_H
#define ROLES_H

#include <string>
#include <sstream>
#include <vector>
#include <tr1/unordered_set>

#include "postgreSQL.h"

#define DARK_SPACE_BOT			0
#define SPAM_BOT			1
#define MAIL_SERVER_BOT			2
#define WEB_SERVER_BOT			3
#define MULTIMEDIA_P2P_NODE		4
#define FTP_BRUTE_FORCER		5
#define FTP_BRUTE_FORCED		6
#define SSH_BRUTE_FORCER		7
#define SSH_BRUTE_FORCED		8
#define TELNET_BRUTE_FORCER		9
#define TELNET_BRUTE_FORCED		10
#define SCAN_BOT			11
#define	UNCLASSIFIED_P2P_NODE		12
#define ENCRYPTED_P2P_NODE		13
#define MICROSOFT_SQL_BRUTE_FORCER	14
#define MICROSOFT_SQL_BRUTE_FORCED	15
#define ORACLE_SQL_BRUTE_FORCER		16
#define ORACLE_SQL_BRUTE_FORCED		17
#define MYSQL_BRUTE_FORCER		18
#define MYSQL_BRUTE_FORCED		19
#define POSTGRESQL_BRUTE_FORCER		20
#define POSTGRESQL_BRUTE_FORCED		21

class Role {
  public:
    uint8_t _role;
    bool _dangerous;
    uint32_t _numHosts;
    uint64_t _numBytes;
    uint32_t _startTime;
    uint32_t _endTime;
    bool _excludePort;
    uint16_t _excludedPort;
    Role(uint8_t role, bool dangerous, uint32_t numHosts, uint64_t numBytes,
         uint32_t startTime, uint32_t endTime);
    Role(uint8_t role, bool dangerous, uint32_t numHosts, uint64_t numBytes,
         uint32_t startTime, uint32_t endTime, uint16_t excludedPort);
};

uint32_t getRoleStartTime(PGconn *postgreSQL, const std::string schemaName,
                          const std::string date, const std::string columnName,
                          const uint32_t &ip);
uint32_t getRoleEndTime(PGconn *postgreSQL, const std::string schemaName,
                        const std::string date, const std::string columnName,
                        const uint32_t &ip);
std::tr1::unordered_set <uint32_t> getCommChannels(PGconn *postgreSQL,
                                                   const std::string date,
                                                   const uint32_t &ip,
                                                   const uint32_t &startTime);
std::tr1::unordered_set <uint32_t> getCommChannels(PGconn *postgreSQL,
                                                   const std::string date,
                                                   const uint32_t &ip,
                                                   const uint32_t &startTime,
                                                   const uint16_t &excludedPort);
std::tr1::unordered_set <uint32_t> _getCommChannels(PGconn *postgreSQL,
                                                    const std::string date,
                                                    const uint32_t &ip,
                                                    const uint32_t &startTime,
                                                    const bool &excludePort,
                                                    const uint16_t &excludedPort);

#endif
