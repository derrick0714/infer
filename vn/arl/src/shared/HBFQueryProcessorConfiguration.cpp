#include "HBFQueryProcessorConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

HBFQueryProcessorConfiguration::
HBFQueryProcessorConfiguration(const fs::path &fileName)
	:SynappConfiguration(fileName),
	_maxMTU(0),
	_maxFlows(0),
	_threadCount(0)
{
	setOptionsDescriptions();
	parseOptions();
}

void HBFQueryProcessorConfiguration::setOptionsDescriptions() {
	options.add_options()
		(
			"max-mtu",
			boost::program_options::value <uint16_t>(),
			"maximum MTU of all monitored network segments"
		)
		(
			"max-flows",
			boost::program_options::value <size_t>(),
			"maximum number of HBF flows to keep in memory"
		)
		(
			"thread-count",
			boost::program_options::value <size_t>(),
			"the number of threads to use for processing HBFs"
		)
	;
}

void HBFQueryProcessorConfiguration::parseOptions() {
	boost::program_options::variables_map vals;
	if (!_parseOptions(vals)) {
		return;
	}

	if (!vals.count("max-mtu")) {
		_errorMsg.assign("max-mtu not specified.");
		_error = true;
		return;
	}
	_maxMTU = vals["max-mtu"].as<uint16_t>();

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

	if (!vals.count("thread-count")) {
		_errorMsg.assign("thread-count not specified.");
		_error = true;
		return;
	}
	_threadCount = vals["thread-count"].as<size_t>();
	if (_threadCount == 0) {
		_errorMsg.assign("thread-count cannot be 0.");
		_error = true;
		return;
	}
}

uint16_t HBFQueryProcessorConfiguration::maxMTU() const {
	return _maxMTU;
}

size_t HBFQueryProcessorConfiguration::maxFlows() const {
	return _maxFlows;
}

size_t HBFQueryProcessorConfiguration::threadCount() const {
	return _threadCount;
}

} // namespace vn
} // namespace arl
} // namespace shared
