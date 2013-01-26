#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include <libpq-fe.h>
#include <sstream>
#include <vector>
#include <set>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <stdarg.h>
#include <netinet/in.h>

#define PG_NULL					0
#define	PG_FIRST_DIMENSION_UNSIGNED		1
#define	PG_SECOND_DIMENSION_UNSIGNED		2
#define PG_FIRST_DIMENSION_INT16		4
#define	PG_SECOND_DIMENSION_INT16		8
#define	PG_FIRST_DIMENSION_INT32		16
#define	PG_SECOND_DIMENSION_INT32		32
#define	PG_FIRST_DIMENSION_INT64		64
#define	PG_SECOND_DIMENSION_INT64		128
#define	PG_FIRST_DIMENSION_DOUBLE		256
#define PG_SECOND_DIMENSION_DOUBLE		512
#define PG_FIRST_DIMENSION_STRING		1024
#define PG_SECOND_DIMENSION_STRING		2048
#define	PG_FIRST_DIMENSION_VECTOR		4096
#define PG_SECOND_DIMENSION_VECTOR		8192
#define PG_FIRST_DIMENSION_UNORDERED_SET	16384
#define PG_SECOND_DIMENSION_UNORDERED_SET	32768
#define PG_FIRST_DIMENSION_UNORDERED_MAP	65536

#define DARK_SPACE_SOURCES_SCHEMA_NAME		"DarkSpaceSources"
#define NON_DNS_TRAFFIC_SCHEMA_NAME		"NonDNSTraffic"
#define DATA_SIZE_SCHEMA_NAME "DataSize"
#define DATA_SIZE_TABLE_SCHEMA "\"dataType\" TEXT NOT NULL, \
                                \"size\" uint64 NOT NULL"
#define LIVE_IPS_SCHEMA_NAME "LiveIPs"
#define COMM_CHANNELS_SCHEMA_NAME "CommChannels"
#define STEPPING_STONES_SCHEMA_NAME "SteppingStones"
#define MULE_CONTACTS_SCHEMA_NAME "MuleContacts"
#define INFECTED_IPS_SCHEMA_NAME "InfectedIPs"
#define INFECTED_CONTACTS_SCHEMA_NAME "InfectedContacts"
#define EVASIVE_TRAFFIC_SCHEMA_NAME "EvasiveTraffic"
#define DARK_SPACE_SOURCES_SCHEMA_NAME "DarkSpaceSources"
#define DARK_SPACE_TARGETS_SCHEMA_NAME "DarkSpaceTargets"
#define NON_DNS_TRAFFIC_SCHEMA_NAME "NonDNSTraffic"
#define MALWARE_SOURCES_SCHEMA_NAME "MalwareSources"
#define MALWARE_TARGETS_SCHEMA_NAME "MalwareTargets"
#define HOST_TRAFFIC_SCHEMA_NAME "HostTraffic"
#define BRUTE_FORCERS_SCHEMA_NAME "BruteForcers"
#define REBOOTS_SCHEMA_NAME "Reboots"
#define ROLES_SCHEMA_NAME "Roles"
#define INTERESTING_IPS_SCHEMA_NAME "InterestingIPs"
#define SCANNERS_SCHEMA_NAME "Scanners"
#define SLOWDOWN_SCHEMA_NAME "Slowdown"
#define SERVER_SLOWDOWN_SCHEMA_NAME "ServerSlowdown"
#define SERVER_SLOWDOWN2_SCHEMA_NAME "ServerSlowdown2"
#define SERVER_SLOWDOWN3_SCHEMA_NAME "ServerSlowdown3"
#define SERVER_SLOWDOWN4_SCHEMA_NAME "ServerSlowdown4"
#define CLIENT_SLOWDOWN_SCHEMA_NAME "ClientSlowdown"
#define CLIENT_SLOWDOWN2_SCHEMA_NAME "ClientSlowdown2"
#define CLIENT_SLOWDOWN3_SCHEMA_NAME "ClientSlowdown3"
#define CLIENT_SLOWDOWN4_SCHEMA_NAME "ClientSlowdown4"

class PGBulkInserter {
  public:
    PGBulkInserter(PGconn*, const char*, const char*, size_t&, const char*);
    void setTableName(std::string);
    bool insert(void*, ...);
    size_t size();
    bool flush();
  private:
    PGconn* postgreSQL;
    std::ostringstream query;
    std::string schemaName;
    std::string tableName;
    size_t flushSize;
    std::vector <uint32_t> columnTypes;
    size_t currentRows;
};

void getPGColumnTypes(const char* rowFormat,
                             std::vector <uint32_t> &columnTypes);

template <class T>
void make1DArray(std::ostringstream &query, T &container,
                        bool firstDimension = true);

template <class T>
void make2DArray(std::ostringstream &query, T &container);

void addPGRow(PGconn *postgreSQL, va_list &columns,
                     std::vector <uint32_t> &columnTypes,
                     std::ostringstream &query);

bool insertPGRow(PGconn* postgreSQL, std::string schemaName,
                 std::string tableName, const char* rowFormat, ...);

int existsPGTable(PGconn* postgreSQL, std::string schemaName,
                         std::string tableName);

bool createPGTable(PGconn* postgreSQL, std::string schemaName,
                          std::string tableName, std::string tableSchema);

bool deletePGRows(PGconn* postgreSQL, std::string schemaName,
                         std::string tableName, std::string columnName,
                         std::string value);

bool dropPGTable(PGconn* postgreSQL, std::string schemaName,
                        std::string tableName);

bool preparePGTable(PGconn* postgreSQL, std::string schemaName,
                           std::string tableName, std::string tableSchema,
                           bool drop = true);

bool preparePGTable(PGconn* postgreSQL, std::string schemaName,
                           std::string tableName, std::string tableSchema,
                           std::string columnName, std::string value);

bool createPGIndex(PGconn *postgreSQL, std::string schemaName,
				   std::string tableName, std::string columnName);

std::pair <bool, std::string> getPreviousTable(PGconn *postgreSQL,
                                               const std::string schema,
                                               const std::string table);

#endif
