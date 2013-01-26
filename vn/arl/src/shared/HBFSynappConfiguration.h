#ifndef HBFSYNAPPCONFIGURATION_H
#define HBFSYNAPPCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "SynappConfiguration.h"
#include "TimeStamp.h"

namespace vn {
namespace arl {
namespace shared {

class HBFSynappConfiguration : public SynappConfiguration {
  public:
	explicit HBFSynappConfiguration(const boost::filesystem::path &fileName);

	size_t maxFlows() const;

	TimeStamp idleTimeout() const;

	size_t minPayload() const;

  private:
	void setOptionsDescriptions();
	void parseOptions();

	size_t _maxFlows;
	TimeStamp _idleTimeout;
	size_t _minPayload;
};

} // namespace vn
} // namespace arl
} // namespace shared

#endif
