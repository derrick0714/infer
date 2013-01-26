#include "dnsNameResolutionSource.h"
#include "FlatFileReader.hpp"
#include "DNS.hpp"

namespace ims {
namespace name {

DNSNameResolutionSource::DNSNameResolutionSource() {
	pthread_rwlock_init(&updateLock, NULL);
}

DNSNameResolutionSource::~DNSNameResolutionSource() {
	pthread_rwlock_destroy(&updateLock);
}

NameResolutionSourceType DNSNameResolutionSource::getType() const {
	return NameResolutionSourceType::DNS;
}

void DNSNameResolutionSource::getIPsFromName(IPSet &ips, const std::string &name) const {
	pthread_rwlock_rdlock(&updateLock);
	std::pair <NameMap::const_iterator, NameMap::const_iterator> ii;
	
	ii = nameMap.equal_range(name);
	for (NameMap::const_iterator i = ii.first; i != ii.second; ++i) {
		ips.insert(i -> second);
	}
	pthread_rwlock_unlock(&updateLock);
}

void DNSNameResolutionSource::getNamesFromIP(NameSet &names, const uint32_t &ip) const {
	pthread_rwlock_rdlock(&updateLock);
	std::pair <IPMap::const_iterator, IPMap::const_iterator> ii;

	ii = ipMap.equal_range(ip);
	for (IPMap::const_iterator i = ii.first; i != ii.second; ++i) {
		names.insert(i -> second);
	}
	pthread_rwlock_unlock(&updateLock);
}

bool DNSNameResolutionSource::hasMapping(const std::string &name) const {
	pthread_rwlock_rdlock(&updateLock);
	if (nameMap.find(name) != nameMap.end()) {
		pthread_rwlock_unlock(&updateLock);
		return true;
	}
	pthread_rwlock_unlock(&updateLock);
	
	return false;
}

bool DNSNameResolutionSource::hasMapping(const uint32_t &ip) const {
	pthread_rwlock_rdlock(&updateLock);
	if (ipMap.find(ip) != ipMap.end()) {
		pthread_rwlock_unlock(&updateLock);
		return true;
	}
	pthread_rwlock_unlock(&updateLock);

	return false;
}

void DNSNameResolutionSource::getResolvedIPs(IPSet &resolvedIPs) const {
	pthread_rwlock_rdlock(&updateLock);
	for (IPMap::const_iterator i = ipMap.begin(); i != ipMap.end(); ++i) {
		resolvedIPs.insert(i -> first);
	}
	pthread_rwlock_unlock(&updateLock);
}

void DNSNameResolutionSource::getMappings(NameIPMap &mappings) const {
	pthread_rwlock_rdlock(&updateLock);
	for (NameMap::const_iterator i = nameMap.begin();
		 i != nameMap.end();
		 ++i)
	{
		mappings.insert(std::make_pair(i -> first, i -> second));
	}
	pthread_rwlock_unlock(&updateLock);
}

bool DNSNameResolutionSource::hasHostResolvedIP(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const {
	HostMaps::const_iterator i;
	HostMap::const_iterator j;
	ResolvedMap::const_iterator k;
	TimeStampSet::const_iterator l;

	pthread_rwlock_rdlock(&updateLock);

	for (i = hostMaps.begin();
		 i != hostMaps.end();
		 ++i)
	{
		j = i -> second -> find(host);
		if (j == i -> second -> end()) {
			continue;
		}
		k = j -> second -> find(resolvedIP);
		if (k == j -> second -> end()) {
			continue;
		}
		l = k -> second -> find(t);
		if (l != k -> second -> end()) {
			pthread_rwlock_unlock(&updateLock);
			return true;
		}
	}

	pthread_rwlock_unlock(&updateLock);
	return false;
}

TimeStamp DNSNameResolutionSource::getLastResolutionTime(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const {
	HostMaps::const_iterator i;
	HostMap::const_iterator j;
	ResolvedMap::const_iterator k;
	TimeStampSet::const_iterator l;
	TimeStamp ret(0, 0);

	pthread_rwlock_rdlock(&updateLock);

	for (i = hostMaps.begin();
		 i != hostMaps.end();
		 ++i)
	{
		j = i -> second -> find(host);
		if (j == i -> second -> end()) {
			continue;
		}
		k = j -> second -> find(resolvedIP);
		if (k == j -> second -> end()) {
			continue;
		}
		l = k -> second -> upper_bound(t);
		if (l == k -> second -> end()) {
			ret = *(--l);
			if (ret == t) {
				break;
			}
		} else if (l == k -> second -> begin()) {
			break;
		} else {
			ret = *(--l);
			break;
		}		
	}

	pthread_rwlock_unlock(&updateLock);
	return ret;
}
	
bool DNSNameResolutionSource::updateMappings(const boost::filesystem::path &file) {
	uint32_t ip;
	std::tr1::unordered_set <std::string> names;
	std::tr1::unordered_set <uint32_t> ips;
	bool isInSet, isFirstIP;

	std::pair <NameMap::iterator, NameMap::iterator> ii;
	NameMap::iterator k;
	std::tr1::unordered_set <std::string>::iterator i;
	std::tr1::unordered_set <uint32_t>::iterator j;
	HostMaps::iterator hostMapsIt;
	HostMap::iterator hostMapIt;
	HostMap *hostMap;
	ResolvedMap::iterator resolvedMapIt;
	ResolvedMap *resolvedMap;
	TimeStampSet *timeStampSet;

	FlatFileReader<DNS> reader;
	DNS dns;
	ErrorStatus errorStatus;

	if (reader.open(file) != E_SUCCESS) {
		return false;
	}

	// get a pointer to the proper Host map
	hostMapsIt = hostMaps.find(file);
	if (hostMapsIt == hostMaps.end()) {
		hostMapsIt = hostMaps.insert(make_pair(file, new HostMap)).first;
	} else {
		//std::cerr << "DEBUG: already loaded..." << std::endl;
		return true;
	}
	hostMap = hostMapsIt -> second;

	pthread_rwlock_wrlock(&updateLock);
	//while (berkeleyDB.read(size, data, hour)) {
	while ((errorStatus = reader.read(dns)) == E_SUCCESS) {
		if (dns.responses().size() == 0) {
			continue;
		}

		// process this dns entry...

		names.clear();
		ips.clear();

		for (std::vector<DNS::DNSResponse*>::const_iterator it(dns.responses().begin());
			 it != dns.responses().end();
			 ++it)
		{
			switch ((*it)->type()) {
			  case 1: // A_RECORD:
				if (names.find((*it)->name()) == names.end()) {
					names.insert((*it)->name());
				}
				ip = ntohl(*reinterpret_cast<const uint32_t *>((*it)->resourceData().data()));
				if (ips.find(ip) == ips.end()) {
					ips.insert(ip);
				}
				break;
			  case 5: // CNAME_RECORD:
				if (names.find((*it)->name()) == names.end()) {
					names.insert((*it)->name());
				}
				if (names.find((*it)->resourceData()) == names.end()) {
					names.insert((*it)->resourceData());
				}
				break;
			  default:
				break;
			}
		}

		// got all responses for this query...put them in the maps....
		isFirstIP = true;

		if (ips.size()) {
			hostMapIt = hostMap -> find(dns.clientIP());
			if (hostMapIt == hostMap -> end()) {
				hostMapIt = hostMap -> insert(make_pair(dns.clientIP(), new ResolvedMap)).first;
			}
			resolvedMap = hostMapIt -> second;
		}

		for (i = names.begin();
			 i != names.end();
			 ++i)
		{
			ii = nameMap.equal_range(*i);
			for (j = ips.begin();
				 j != ips.end();
				 ++j)
			{
				// check if <i,j> is in the map.
				//if (std::find_if(ii.first, ii.second, bind2nd(
				isInSet = false;
				for (k = ii.first; k != ii.second; ++k) {
					if (k -> second == *j) {
						isInSet = true;
						break;
					}
				}

				if (!isInSet) {
					// add this pair to the maps
					nameMap.insert(std::make_pair(*i, *j));
					ipMap.insert(std::make_pair(*j, *i));
				}
				if (isFirstIP) {
					// insert the ips into the ResolvedMap
					resolvedMapIt = resolvedMap -> find(*j);
					if (resolvedMapIt == resolvedMap -> end()) {
						resolvedMapIt = resolvedMap -> insert(make_pair(*j, new TimeStampSet)).first;
					}
					timeStampSet = resolvedMapIt -> second;
					
					timeStampSet -> insert(dns.responseTime());
				}
			}
			isFirstIP = false;
		}
	}
	reader.close();

	pthread_rwlock_unlock(&updateLock);

	return true;
}

void DNSNameResolutionSource::clear() {
	pthread_rwlock_wrlock(&updateLock);

	nameMap.clear();
	ipMap.clear();
	//clear HostMaps....
	HostMaps::iterator i;
	HostMap::iterator j;
	ResolvedMap::iterator k;

	for (i = hostMaps.begin(); i != hostMaps.end(); ++i) {
		for (j = i -> second -> begin(); j != i -> second -> end(); ++j) {
			for (k = j -> second -> begin(); k != j -> second -> end(); ++k) {
				delete k -> second;
			}
			delete j -> second;
		}
		delete i -> second;
	}
	hostMaps.clear();

	pthread_rwlock_unlock(&updateLock);
}

}
}
