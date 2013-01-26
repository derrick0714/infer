#ifndef _TEST_NAME_RESOLUTION_SOURCE_H_
#define _TEST_NAME_RESOLUTION_SOURCE_H_

#include <tr1/unordered_set>

#include "nameResolutionSource.hpp"

namespace ims {
  namespace name {
    class TestNameResolutionSource : public NameResolutionSource {
      public:
	void getIPsFromName(std::tr1::unordered_set <uint32_t> &ips, const std::string &name) const;
	void getNamesFromIP(std::tr1::unordered_set <std::string> &names, const uint32_t &ip) const;
	bool hasMapping(const std::string &name) const;
	bool hasMapping(const uint32_t &ip) const;
	bool updateMappings(const std::string &bdbFilename);
    };
  }
}

#endif
