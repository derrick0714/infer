#ifndef SAMPLESYNAPP_H
#define SAMPLESYNAPP_H

#include "../shared/Synapp.hpp"
#include "../shared/DefaultSynappArguments.h"

namespace vn {
namespace arl {
namespace synapps {

class SampleSynapp : public shared::Synapp {
  public:
	explicit SampleSynapp(const shared::DefaultSynappArguments &args);

	int run();

  private:
	const shared::DefaultSynappArguments args;
};

} // namespace synapps
} // namespace arl
} // namespace synapps

#endif
