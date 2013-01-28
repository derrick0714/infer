#ifndef INFER_BIN_DNS2PSQL_DNS_RESPONSE_ROW_HPP_
#define INFER_BIN_DNS2PSQL_DNS_RESPONSE_ROW_HPP_

#include <string>

struct dns_response_row {
	int64_t dns_id;
	std::string name;
	uint16_t type;
	std::string resource_data;
	int32_t ttl;
};

#endif
