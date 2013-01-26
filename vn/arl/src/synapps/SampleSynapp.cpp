#include <boost/date_time/posix_time/posix_time.hpp>

#include "SampleSynapp.h"
#include "../shared/SampleSynappConfiguration.h"

namespace vn {
namespace arl {
namespace synapps {

SampleSynapp::SampleSynapp(const shared::DefaultSynappArguments &args)
	:Synapp(),
	 args(args)
{
}

int SampleSynapp::run() {
	if (!args) {
		error << "Error: " << args.error() << std::endl << std::endl
			  << args << std::endl;

		return 1;
	}

	shared::SampleSynappConfiguration conf(args.configFile());
	if (!conf) {
		error << "Error: Configuration: " << conf.error() << std::endl
			  << conf << std::endl;
		return 1;
	}

	if (args.help()) {
		debug << args << std::endl;
		return 0;
	}

	using namespace boost::posix_time;

	debug << "Command line arguments:" << std::endl;
	debug << "\tStart Time:  " << static_cast <ptime>(args.startTime())
															<< std::endl;
	debug << "\tEnd Time:    " << static_cast <ptime>(args.endTime())
															<< std::endl;
	debug << "\tInput Dir:   " << args.inputDir() << std::endl;
	debug << "\tOutput Dir:  " << args.outputDir() << std::endl;
	debug << "\tConfig File: " << args.configFile() << std::endl;
	debug << "\tOutput File: " << args.outputFile() << std::endl;
	debug << std::endl;
	debug << "Configuration file options:" << std::endl;
	debug << "\tDB Timeout:  " << conf.dbTimeout() << std::endl;

	return 0;
}

} // namespace synapps
} // namespace arl
} // namespace synapps
