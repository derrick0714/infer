#include "SampleSynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

SampleSynappConfiguration::SampleSynappConfiguration(const boost::filesystem::path &fileName)
	:SynappConfiguration(fileName),
	 _dbTimeout(0)
{
	setOptionsDescriptions();
	parseOptions();
}

void SampleSynappConfiguration::setOptionsDescriptions() {
	options.add_options()
		(
			"db-timeout",
			boost::program_options::value <uint32_t>(),
			"BerkeleyDB timeout"
		)
	;
}

void SampleSynappConfiguration::parseOptions() {
	boost::program_options::variables_map vals;
	if (!_parseOptions(vals)) {
		return;
	}

	if (!vals.count("db-timeout")) {
		_errorMsg.assign("db-timeout not specified.");
		_error = true;
		return;
	}
	_dbTimeout = vals["db-timeout"].as<uint32_t>();
}

uint32_t SampleSynappConfiguration::dbTimeout() const {
	return _dbTimeout;
}

} // namespace vn
} // namespace arl
} // namespace shared
