#include "HBFSynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

HBFSynappConfiguration::HBFSynappConfiguration(const boost::filesystem::path &fileName)
	:SynappConfiguration(fileName),
	 _maxFlows(0),
	 _idleTimeout(),
	 _minPayload(0)
{
	setOptionsDescriptions();
	parseOptions();
}

void HBFSynappConfiguration::setOptionsDescriptions() {
	options.add_options()
		(
			"max-flows",
			boost::program_options::value <size_t>(),
			"maximum number of HBF flows to keep in memory"
		)
		(
			"idle-timeout",
			boost::program_options::value <uint32_t>(),
			"the number of seconds a flow can remain idle without being "
			"flushed to disk"
		)
		(
			"min-payload",
			boost::program_options::value <size_t>(),
			"the minimum captured payload size for a packet to be used"
		)
	;
}

void HBFSynappConfiguration::parseOptions() {
	boost::program_options::variables_map vals;
	if (!_parseOptions(vals)) {
		return;
	}

	if (!vals.count("max-flows")) {
		_errorMsg.assign("max-flows not specified.");
		_error = true;
		return;
	}
	_maxFlows = vals["max-flows"].as<size_t>();
	if (_maxFlows == 0) {
		_errorMsg.assign("max-flows cannot be 0.");
		_error = true;
		return;
	}

	if (!vals.count("idle-timeout")) {
		_error = true;
		_errorMsg.assign("idle-timeout not specified.");
		return;
	}
	_idleTimeout.set(vals["idle-timeout"].as<uint32_t>(), 0);

	if (vals.count("min-payload")) {
		_minPayload = vals["min-payload"].as<size_t>();
	}
}

size_t HBFSynappConfiguration::maxFlows() const {
	return _maxFlows;
}

TimeStamp HBFSynappConfiguration::idleTimeout() const {
	return _idleTimeout;
}

size_t HBFSynappConfiguration::minPayload() const {
	return _minPayload;
}

} // namespace vn
} // namespace arl
} // namespace shared
