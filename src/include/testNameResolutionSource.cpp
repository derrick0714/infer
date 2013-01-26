#include "testNameResolutionSource.h"

namespace ims {
  namespace name {
    void TestNameResolutionSource::getIPsFromName(std::tr1::unordered_set <uint32_t> &ips, const std::string &name) const {}

    void TestNameResolutionSource::getNamesFromIP(std::tr1::unordered_set <std::string> &names, const uint32_t &ip) const {}

    bool TestNameResolutionSource::hasMapping(const std::string &name) const {
	return false;
    }

    bool TestNameResolutionSource::hasMapping(const uint32_t &ip) const {
	return false;
    }

    bool TestNameResolutionSource::updateMappings(const std::string &bdbFilename) {
	return false;
    }
  }
}
