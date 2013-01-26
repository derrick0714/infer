/* 
 * File:   DarkAccessArguments.cpp
 * Author: Mike
 * 
 * Created on December 2, 2009, 10:27 PM
 */

#include "DarkAccessArguments.h"
#include "DarkAccessParams.h"
#include "../SymptomDefaults.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    DarkAccessArguments::DarkAccessArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
	    _configFile(CONFIG_FILE),
	    _help(false),
	    _stdout(true),
	    sensor(SENSOR_NAME),
	    hours(DEFAULT_TW_VALUE),
	    _stateFile(DEFAULT_STATE_FILE) {
		setOptionsDescriptions();
		parseOptions();
	    }

	    void DarkAccessArguments::setOptionsDescriptions() {
		using namespace boost::program_options;

		options_description reqArgs("Symptom Required Arguments");
		options_description optArgs("Symptom Optional Arguments");

		reqArgs.add_options()
			(
			MAKE_CMD(OPT_INPUT_NETFLOW, OPT_SHORT_INPUT_NETFLOW),
			value <string> (),
			"Netflow input file"
			)
			(
			MAKE_CMD(OPT_OUTFILE, OPT_SHORT_OUTFILE),
			value<string>(),
			"Output file"
			);

		optArgs.add_options()
			(
			MAKE_CMD(OPT_CONFIG_FILE, OPT_SHORT_CONFIG_FILE),
			boost::program_options::value <string > (),
			"file from which to read additional configuration options."
			)
			(
			MAKE_CMD(OPT_HELP, OPT_SHORT_HELP),
			"prints a help message to stdout"
			)
			(
			MAKE_CMD(OPT_STDOUT, OPT_SHORT_STDOUT),
			"do NOT output to stdout. Default on."
			)
			(
			MAKE_CMD(OPT_TW, OPT_SHORT_TW),
			boost::program_options::value <int> (),
			"Specify the time window for the Darkspace Access scan. Default is 24"
			)
			(
			MAKE_CMD(OPT_ZONE, OPT_SHORT_ZONE),
			boost::program_options::value <string> (),
			"Specify the monitored zone. ex: 192.168.1.2/24 or 2001:480:60:55::3/96"
			)
			(
			MAKE_CMD(OPT_STATE_FILE, OPT_SHORT_STATE_FILE),
			boost::program_options::value <string> (),
			"Specify the state file from previous execution or a new file"
			)
			(
			MAKE_CMD(OPT_SENSOR, OPT_SHORT_SENSOR),
			boost::program_options::value <string> (),
			"Specify the sensor name, optional but recommended as NetFlow file doesn't contain this name."
			);

		arguments.add(reqArgs);
		arguments.add(optArgs);
	    }

	    const boost::filesystem::path & DarkAccessArguments::inputNetFlowFile() const {
		return _inputDir;
	    }

	    const boost::filesystem::path & DarkAccessArguments::configFile() const {
		return _configFile;
	    }

	    const boost::filesystem::path& DarkAccessArguments::outputFile() const {
		return _output;
	    }

	    const int DarkAccessArguments::getTimeWindow() const {
		return hours;
	    }

	    const IPZone DarkAccessArguments::getMonitoredZone() const {
		return zone;
	    }

	    const boost::filesystem::path& DarkAccessArguments::getStateFile() const {
		return _stateFile;
	    }

	    const string DarkAccessArguments::getSensorName() const {
		return sensor;
	    }

	    bool DarkAccessArguments::isStdOut() const {
		return _stdout;
	    }

	    bool DarkAccessArguments::isHelp() const {
		return _help;
	    }

	    void DarkAccessArguments::parseOptions() {
		boost::program_options::variables_map opts;
		if (!_parseOptions(opts)) {
		    return;
		}

		// optional options
		if (opts.count(OPT_HELP) || opts.size() == 0) {
		    _help = true;
		    return;
		}

		if(opts.count(OPT_STDOUT)) {
		    _stdout = false;
		}

		if(opts.count(OPT_SENSOR)) {
		    sensor = opts[OPT_SENSOR].as<string>();
		}

		if (opts.count(OPT_CONFIG_FILE)) {
		    _configFile = boost::filesystem::path(opts[OPT_CONFIG_FILE].as<string > ());

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

		    DarkAccessConfiguration* config = new DarkAccessConfiguration(_configFile);

		    if(!config) {
			_errorMsg.assign("Error in the configuration file (");
			_errorMsg.append(_configFile.file_string());
			_errorMsg.append("): ");
			_errorMsg.append(config->error());
			_error = true;

			delete config;
			
			return;
		    }

		    hours = config->getTimeWindow();
		    zone = config->getMonitoredZone();
		    _stateFile = config->getStateFile();

		    delete config;
		}

		if(opts.count(OPT_TW)) {
		    hours = opts[OPT_TW].as<int>();
		}

		if(opts.count(OPT_ZONE)) {
		    string zone_str = opts[OPT_ZONE].as<string>();
		    IPsubnet sn = zone.addAddressRangeList(zone_str);

		    if(sn.errored()) {
			_errorMsg.assign("Error on command line argument ").append(OPT_ZONE).append(":");
			_errorMsg.append(sn.getErrorStr());
			_error = true;
			return;
		    }
		}

		if(opts.count(OPT_STATE_FILE)) {
		    _stateFile = boost::filesystem::path(opts[OPT_STATE_FILE].as<string>());
		}

		if (boost::filesystem::exists(_stateFile) && !boost::filesystem::is_regular_file(_stateFile)) {
		    _errorMsg.assign("invalid state file: \"");
		    _errorMsg.append(_stateFile.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

		// verify
		if(hours <= 0) {
		    _errorMsg.assign("Time window value must be at least 1");
		    _error = true;
		    return;
		}

		if(zone.getEntryCount() == 0) {
		    _errorMsg.assign("The monitired zone must be speficied either in the configuration file or on the command line");
		    _error = true;
		    return;
		}

		// required opts
		if (!opts.count(OPT_INPUT_NETFLOW)) {
		    _errorMsg.assign(string(OPT_INPUT_NETFLOW) + " not specified.");
		    _error = true;
		    return;
		}
		_inputDir = boost::filesystem::path(opts[OPT_INPUT_NETFLOW].as<std::string > ());
		if (!boost::filesystem::exists(_inputDir)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputDir.file_string());
		    _errorMsg.append("\" does not exist.");
		    _error = true;
		    return;
		}
		if (!boost::filesystem::is_regular_file(_inputDir)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputDir.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

		if(!opts.count(OPT_OUTFILE)) {
		    _errorMsg.assign(string(OPT_OUTFILE) + " not specified.");
		    _error = true;
		    return;
		}
		_output = boost::filesystem::path(opts[OPT_OUTFILE].as<std::string > ());
		if (boost::filesystem::exists(_output)) {
		    cerr << "WARN: File \"" << _output.file_string()
		         << "\" already exists. Data will be appended." << endl;
		}
	    }
	}
    }
}
