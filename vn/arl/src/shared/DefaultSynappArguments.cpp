#include <time.h>

#include "DefaultSynappArguments.h"

namespace vn {
namespace arl {
namespace shared {

DefaultSynappArguments::DefaultSynappArguments(int argc, char **argv)
	:SynappArguments(argc, argv),
	 _startTime(),
	 _endTime(),
	 _inputDir(),
	 _outputDir(),
	 _configFile(boost::filesystem::path(argv[0]).replace_extension(".conf")),
	 _outputFile(),
	 _help(false)
{
	setOptionsDescriptions();
	parseOptions();
}

const TimeStamp & DefaultSynappArguments::startTime() const {
	return _startTime;
}

const TimeStamp & DefaultSynappArguments::endTime() const {
	return _endTime;
}

const boost::filesystem::path & DefaultSynappArguments::inputDir() const {
	return _inputDir;
}

const boost::filesystem::path & DefaultSynappArguments::outputDir() const {
	return _outputDir;
}

const boost::filesystem::path & DefaultSynappArguments::configFile() const {
	return _configFile;
}

const boost::filesystem::path & DefaultSynappArguments::outputFile() const {
	return _outputFile;
}

bool DefaultSynappArguments::help() const {
	return _help;
}

void DefaultSynappArguments::setOptionsDescriptions() {
	boost::program_options::options_description requiredArgs
												("Synapp Required Arguments");
	requiredArgs.add_options()
		(
			"start-time,s",
			boost::program_options::value <std::string>(),
			"start time of data to process"
		)
		(
			"end-time,e",
			boost::program_options::value <std::string>(),
			"end time of data to process"
		)
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
			"output-file,f",
			boost::program_options::value <std::string>(),
			"a file name to which output should be written. "
										"Causes --output-dir to be ignored"
		)
		(
			"help,h",
			"prints a help message to stdout"
		)
	;

	arguments.add(requiredArgs);
	arguments.add(optionalArgs);
}

void DefaultSynappArguments::parseOptions() {
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

	if (opts.count("output-file")) {
		_outputFile = boost::filesystem::path
								(opts["output-file"].as<std::string>());
		if (boost::filesystem::exists(_outputFile)) {
			_errorMsg.assign("invalid output-file: \"");
			_errorMsg.append(_outputFile.file_string());
			_errorMsg.append("\" already exists.");
			_error = true;
			return;
		}
		if (_outputFile.parent_path() != boost::filesystem::path()) {
			if (!boost::filesystem::exists(_outputFile.parent_path())) {
				_errorMsg.assign("invalid output-file: directory \"");
				_errorMsg.append(_outputFile.parent_path().file_string());
				_errorMsg.append("\" does not exist.");
				_error = true;
				return;
			}
			if (!boost::filesystem::is_directory(_outputFile.parent_path())) {
				_errorMsg.assign("invalid output-file: \"");
				_errorMsg.append(_outputFile.parent_path().file_string());
				_errorMsg.append("\" is not a directory.");
				_error = true;
				return;
			}
		}
	}

	// required opts
	if (!opts.count("start-time")) {
		_errorMsg.assign("start-time not specified.");
		_error = true;
		return;
	}
	struct tm tmpTime;
	if (strptime(opts["start-time"].as<std::string>().c_str(), 
				 "%a %b %d %H:%M:%S %Z %Y",
				 &tmpTime) == NULL)
	{
		_errorMsg.assign("invalid start-time.");
		_error = true;
		return;
	}
	time_t tmpTimet = timegm(&tmpTime);
	_startTime.set(static_cast <uint32_t>(tmpTimet), 0);

	if (!opts.count("end-time")) {
		_errorMsg.assign("end-time not specified.");
		_error = true;
		return;
	}
	if (strptime(opts["end-time"].as<std::string>().c_str(), 
				 "%a %b %d %H:%M:%S %Z %Y",
				 &tmpTime) == NULL)
	{
		_errorMsg.assign("invalid end-time.");
		_error = true;
		return;
	}
	tmpTimet = timegm(&tmpTime);
	_endTime.set(static_cast <uint32_t>(tmpTimet), 0);

	if (_endTime <= _startTime) {
		_errorMsg.assign("invalid time range: end-time <= start-time.");
		_error = true;
		return;
	}

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

	if (opts.count("output-file")) {
		// only parse output-dir if output-file is not specified
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
