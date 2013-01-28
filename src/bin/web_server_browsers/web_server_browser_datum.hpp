#ifndef INFER_BIN_WEB_SERVER_BROWSERS_WEB_SERVER_BROWSER_DATUM_HPP_
#define INFER_BIN_WEB_SERVER_BROWSERS_WEB_SERVER_BROWSER_DATUM_HPP_

#include <string>

#include "timeStamp.h"

struct web_server_browser_datum {
	uint32_t server_ip;
	std::pair<std::string, std::string> browser;
	int64_t request_count;
	TimeStamp first_request_time;
	TimeStamp last_request_time;

	bool operator < (const web_server_browser_datum &rhs) const {
		return request_count < rhs.request_count;
	}
};

#endif
