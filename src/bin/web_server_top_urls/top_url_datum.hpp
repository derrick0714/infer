#ifndef INFER_BIN_WEB_SERVER_TOP_URLS_TOP_URL_DATUM_HPP_
#define INFER_BIN_WEB_SERVER_TOP_URLS_TOP_URL_DATUM_HPP_

#include <string>

#include "timeStamp.h"

struct top_url_datum {
	uint32_t server_ip;
	std::string url;
	int64_t request_count;
	TimeStamp first_request_time;
	TimeStamp last_request_time;

	bool operator < (const top_url_datum &rhs) const {
		return request_count < rhs.request_count;
	}
};

#endif
