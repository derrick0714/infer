#ifndef SAMPLESYNAPPCONFIGURATION_H
#define SAMPLESYNAPPCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "SynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

class SampleSynappConfiguration : public SynappConfiguration {
  public:
	explicit SampleSynappConfiguration(const boost::filesystem::path &fileName);

	uint32_t dbTimeout() const;

  private:
	void setOptionsDescriptions();
	void parseOptions();

	uint32_t _dbTimeout;
};

} // namespace vn
} // namespace arl
} // namespace shared

#endif
