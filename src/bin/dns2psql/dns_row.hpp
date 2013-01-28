#ifndef INFER_BIN_DNS2PSQL_DNS_ROW_HPP_
#define INFER_BIN_DNS2PSQL_DNS_ROW_HPP_

#include <string>

#include "timeStamp.h"

struct dns_row {
	int64_t id;
	TimeStamp query_time;
	TimeStamp response_time;
	uint32_t client_ip;
	uint32_t server_ip;
	std::string query_name;
	uint16_t query_type;
	uint16_t response_code;
};

#endif
