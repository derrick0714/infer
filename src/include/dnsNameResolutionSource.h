#ifndef _DNS_NAME_RESOLUTION_SOURCE_H_
#define _DNS_NAME_RESOLUTION_SOURCE_H_

#include <pthread.h>
#include <map>
#include <set>

#include "nameResolutionSource.hpp"
#include "timeStamp.h"

namespace ims {
namespace name {

class DNSNameResolutionSource : public NameResolutionSource {
  public:
	DNSNameResolutionSource();
	~DNSNameResolutionSource();

	void getIPsFromName(IPSet &ips, const std::string &name) const;
	void getNamesFromIP(NameSet &names, const uint32_t &ip) const;
	bool hasMapping(const std::string &name) const;
	bool hasMapping(const uint32_t &ip) const;
	void getResolvedIPs(IPSet &resolvedIPs) const;
	void getMappings(NameIPMap &mappings) const;
	bool hasHostResolvedIP(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const;
	TimeStamp getLastResolutionTime(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const;

	NameResolutionSourceType getType() const;

	bool updateMappings(const boost::filesystem::path &file);
	void clear();

  private:
	typedef std::multimap <std::string, uint32_t> NameMap;
	typedef std::multimap <uint32_t, std::string> IPMap;
	typedef std::set <TimeStamp> TimeStampSet;
	typedef std::map <uint32_t, TimeStampSet *> ResolvedMap;
	typedef std::map <uint32_t, ResolvedMap *> HostMap;
	typedef std::map <boost::filesystem::path, HostMap *> HostMaps;

	NameMap nameMap;
	IPMap ipMap;
	HostMaps hostMaps;
	mutable pthread_rwlock_t updateLock;
};

}
}

#endif
