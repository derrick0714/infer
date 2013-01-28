#ifndef INFER_BIN_WEB_SERVER_SERVERS_WEB_SERVER_SERVER_DATUM_HPP_
#define INFER_BIN_WEB_SERVER_SERVERS_WEB_SERVER_SERVER_DATUM_HPP_

#include <string>

#include "timeStamp.h"

struct web_server_server_datum {
	uint32_t server_ip;
	std::string server;
	int64_t response_count;
	TimeStamp first_response_time;
	TimeStamp last_response_time;

	bool operator < (const web_server_server_datum &rhs) const {
		return response_count < rhs.response_count;
	}
};

#endif
