#ifndef HBFQUERYPROCESSORCONFIGURATION_H
#define HBFQUERYPROCESSORCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "SynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

class HBFQueryProcessorConfiguration : public SynappConfiguration {
  public:
	explicit HBFQueryProcessorConfiguration(const fs::path &fileName);

	uint16_t maxMTU() const;
	
	size_t maxFlows() const;

	size_t threadCount() const;

  private:
	void setOptionsDescriptions();
	void parseOptions();

	uint16_t _maxMTU;
	size_t _maxFlows;
	size_t _threadCount;
};

} // namespace vn
} // namespace arl
} // namespace shared

#endif
