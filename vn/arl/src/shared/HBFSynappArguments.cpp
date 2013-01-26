#include <time.h>

#include "HBFSynappArguments.h"

namespace vn {
namespace arl {
namespace shared {

HBFSynappArguments::HBFSynappArguments(int argc, char **argv)
	:SynappArguments(argc, argv),
	 _inputDir(),
	 _outputDir(),
	 _configFile("/usr/local/etc/synapp_hbf.conf"),
	 _help(false)
{
	setOptionsDescriptions();
	parseOptions();
}

const boost::filesystem::path & HBFSynappArguments::inputDir() const {
	return _inputDir;
}

const boost::filesystem::path & HBFSynappArguments::outputDir() const {
	return _outputDir;
}

const boost::filesystem::path & HBFSynappArguments::configFile() const {
	return _configFile;
}

bool HBFSynappArguments::help() const {
	return _help;
}

void HBFSynappArguments::setOptionsDescriptions() {
	boost::program_options::options_description requiredArgs
												("Synapp Required Arguments");
	requiredArgs.add_options()
		(
			"input-dir,i",
			boost::program_options::value <std::string>(),
			"directory from which to read data files")
		(
			"output-dir,o",
			boost::program_options::value <std::string>(),
			"directory in which to write synopses"
		)
	;

	boost::program_options::options_description optionalArgs
												("Synapp Optional Arguments");
	optionalArgs.add_options()
		(
			"config-file,c",
			boost::program_options::value <std::string>(),
			"file from which to read additional configuration options."
		)
		(
			"help,h",
			"prints a help message to stdout"
		)
	;

	arguments.add(requiredArgs);
	arguments.add(optionalArgs);
}

void HBFSynappArguments::parseOptions() {
	boost::program_options::variables_map opts;
	if (!_parseOptions(opts)) {
		return;
	}

	// optional options
	if (opts.count("help")) {
		_help = true;
		return;
	}

	if (opts.count("config-file")) {
		_configFile = boost::filesystem::path
								(opts["config-file"].as<std::string>());
	}
	if (!boost::filesystem::exists(_configFile)) {
		_errorMsg.assign("invalid config-file: file \"");
		_errorMsg.append(_configFile.file_string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_regular_file(_configFile)) {
		_errorMsg.assign("invalid config-file: file \"");
		_errorMsg.append(_configFile.file_string());
		_errorMsg.append("\" is not a regular file.");
		_error = true;
		return;
	}	

	// required opts
	if (!opts.count("input-dir")) {
		_errorMsg.assign("input-dir not specified.");
		_error = true;
		return;
	}
	_inputDir = boost::filesystem::path(opts["input-dir"].as<std::string>());
	if (!boost::filesystem::exists(_inputDir)) {
		_errorMsg.assign("invalid input-dir: directory \"");
		_errorMsg.append(_inputDir.file_string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_directory(_inputDir)) {
		_errorMsg.assign("invalid input-dir: \"");
		_errorMsg.append(_inputDir.file_string());
		_errorMsg.append("\" is not a directory.");
		_error = true;
		return;
	}

	if (!opts.count("output-dir")) {
		_errorMsg.assign("output-dir not specified.");
		_error = true;
		return;
	}
	_outputDir = boost::filesystem::path(opts["output-dir"].as<std::string>());
	if (!boost::filesystem::exists(_outputDir)) {
		_errorMsg.assign("invalid output-dir: directory \"");
		_errorMsg.append(_outputDir.file_string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_directory(_outputDir)) {
		_errorMsg.assign("invalid output-dir: \"");
		_errorMsg.append(_outputDir.file_string());
		_errorMsg.append("\" is not a directory.");
		_error = true;
		return;
	}
}

} // namespace shared
} // namespace arl
} // namespace vn
