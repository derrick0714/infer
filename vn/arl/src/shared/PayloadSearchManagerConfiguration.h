#ifndef PAYLOADSEARCHMANAGERCONFIGURATION_H
#define PAYLOADSEARCHMANAGERCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "SynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

class PayloadSearchManagerConfiguration : public SynappConfiguration {
  public:
	explicit PayloadSearchManagerConfiguration(const fs::path &fileName);

	uint16_t maxMTU() const;
	
	size_t maxFlows() const;

	size_t threadCount() const;

	std::string pgHost() const;
	std::string pgHostaddr() const;
	std::string pgPort() const;
	std::string pgDbname() const;
	std::string pgUser() const;
	std::string pgPassword() const;
	std::string pgConnectTimeout() const;
	std::string pgOptions() const;
	std::string pgSslmode() const;
	std::string pgRequiressl() const;
	std::string pgKrbsrvname() const;
	std::string pgGsslib() const;
	std::string pgService() const;

	std::string pgSchema() const;

  private:
	void setOptionsDescriptions();
	void parseOptions();

	uint16_t _maxMTU;
	size_t _maxFlows;
	size_t _threadCount;

	std::string _pgHost;
	std::string _pgHostaddr;
	std::string _pgPort;
	std::string _pgDbname;
	std::string _pgUser;
	std::string _pgPassword;
	std::string _pgConnectTimeout;
	std::string _pgOptions;
	std::string _pgSslmode;
	std::string _pgRequiressl;
	std::string _pgKrbsrvname;
	std::string _pgGsslib;
	std::string _pgService;

	std::string _pgSchema;
};

} // namespace vn
} // namespace arl
} // namespace shared

#endif
