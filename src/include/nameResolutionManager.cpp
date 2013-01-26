#include <set>

#include "nameResolutionManager.h"

namespace ims {
namespace name {

void NameResolutionManager::addSource(NameResolutionSource *source) {
	sources.insert(std::make_pair(source -> getType(), source));
}

// adds each IP associated with name from the specified sourceType to ips
void NameResolutionManager::getIPsFromName(IPSet &ips, const std::string &name, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		i -> second -> getIPsFromName(ips, name);
	}
}

void NameResolutionManager::getNamesFromIP(NameSet &names, const uint32_t &ip, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		i -> second -> getNamesFromIP(names, ip);
	}
}

bool NameResolutionManager::hasMapping(const std::string &name, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		if (i -> second -> hasMapping(name)) {
			return true;
		}
	}

	return false;
}

bool NameResolutionManager::hasMapping(const uint32_t &ip, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		if (i -> second -> hasMapping(ip)) {
			return true;
		}
	}

	return false;
}

void NameResolutionManager::getResolvedIPs(IPSet &resolvedIPs, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		i -> second -> getResolvedIPs(resolvedIPs);
	}
}

bool NameResolutionManager::hasHostResolvedIP(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		if (i -> second -> hasHostResolvedIP(host, resolvedIP, t)) {
			return true;
		}
	}

	return false;
}
	
TimeStamp NameResolutionManager::getLastResolutionTime(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	TimeStamp ret(0,0), next;
	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		next = i -> second -> getLastResolutionTime(host, resolvedIP, t);
		if (next > ret) {
			ret = next;
		}
	}

	return ret;
}

void NameResolutionManager::getMappings(NameIPMap &mappings, NameResolutionSourceType sourceType) const {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		i -> second -> getMappings(mappings);
	}
}

bool NameResolutionManager::updateMappings(const boost::filesystem::path &file, NameResolutionSourceType sourceType) {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		return false;
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		if (!(i -> second -> updateMappings(file))) {
			return false;
		}
	}

	return true;
}

void NameResolutionManager::clear(NameResolutionSourceType sourceType) {
	std::pair<SourcesMap::const_iterator, SourcesMap::const_iterator> ii;
	
	if (sourceType == NameResolutionSourceType::ANY) {
		ii = std::make_pair(sources.begin(), sources.end());
	} else {
		ii = sources.equal_range(sourceType);
	}

	for (SourcesMap::const_iterator i = ii.first; i != ii.second; ++i) {
		i -> second -> clear();
	}
}

}
}
