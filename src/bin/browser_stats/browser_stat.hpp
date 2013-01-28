#ifndef INFER_BIN_BROSWER_STATS_BROWSER_STAT_HPP_
#define INFER_BIN_BROSWER_STATS_BROWSER_STAT_HPP_

#include <string>

struct browser_stat {
	std::string browser;
	size_t internal_host_count;
};

#endif
