#ifndef INFER_BIN_WEB_SERVER_CRAWLERS_WEB_SERVER_CRAWLER_DATUM_HPP_
#define INFER_BIN_WEB_SERVER_CRAWLERS_WEB_SERVER_CRAWLER_DATUM_HPP_

#include <string>

#include "timeStamp.h"

struct web_server_crawler_datum {
	uint32_t server_ip;
	std::pair<uint32_t, std::string> crawler;
	int64_t request_count;
	TimeStamp first_request_time;
	TimeStamp last_request_time;

	bool operator < (const web_server_crawler_datum &rhs) const {
		return request_count < rhs.request_count;
	}
};

#endif
