#ifndef INFER_BIN_BROSWER_STATS_BROWSER_VERSION_STAT_HPP_
#define INFER_BIN_BROSWER_STATS_BROWSER_VERSION_STAT_HPP_

#include <string>

struct browser_version_stat {
	std::string browser;
	std::string version;
	size_t internal_host_count;
};

#endif
