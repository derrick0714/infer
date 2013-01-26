/* 
 * File:   NetflowViewerArguments.cpp
 * Author: Mike
 * 
 * Created on January 18, 2010, 2:05 PM
 */

#include "NetflowViewerArguments.h"
#include "NetflowViewerParams.h"
#include "../SymptomDefaults.h"
#include "../../shared/TimeStamp.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    NetflowViewerArguments::NetflowViewerArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
	    _help(false),
	    sensor(SENSOR_NAME),
	    _startTime(0, 0),
	    _endTime(0xFFFFFFFF, 0xFFFFFFFF) {
		setOptionsDescriptions();
		parseOptions();
	    }

	    void NetflowViewerArguments::setOptionsDescriptions() {
		using namespace boost::program_options;

		options_description reqArgs("Viewer Required Arguments");
		options_description optArgs("Viewer Optional Arguments");

		reqArgs.add_options()
			(
			MAKE_CMD(OPT_INPUT_FILE, OPT_SHORT_INPUT_FILE),
			value <string > (),
			"input file"
			);

		optArgs.add_options()
			(
			MAKE_CMD(OPT_HELP, OPT_SHORT_HELP),
			"prints a help message to stdout"
			)
			(
			MAKE_CMD(OPT_SENSOR, OPT_SHORT_SENSOR),
			boost::program_options::value <string > (),
			"Specify the sensor name, optional but recommended."
			)
			(
			MAKE_CMD(OPT_START_TIME, OPT_SHORT_START_TIME),
			boost::program_options::value <string > (),
			"start time of data to output"
			)
			(
			MAKE_CMD(OPT_END_TIME, OPT_SHORT_END_TIME),
			boost::program_options::value <string > (),
			"end time of data to output"
			);

		arguments.add(reqArgs);
		arguments.add(optArgs);
	    }

	    const boost::filesystem::path & NetflowViewerArguments::inputFile() const {
		return _inputFile;
	    }

	    const string& NetflowViewerArguments::getSensorName() const {
		return sensor;
	    }

	    const TimeStamp& NetflowViewerArguments::getStartTime() const {
		return _startTime;
	    }

	    const TimeStamp& NetflowViewerArguments::getEndTime() const {
		return _endTime;
	    }

	    bool NetflowViewerArguments::isHelp() const {
		return _help;
	    }

	    bool NetflowViewerArguments::isStdOut() const {
		return true;
	    }

	    void NetflowViewerArguments::parseOptions() {
		boost::program_options::variables_map opts;
		if (!_parseOptions(opts)) {
		    return;
		}

		// optional options
		if (opts.count(OPT_HELP)) {
		    _help = true;
		    return;
		}

		if (opts.count(OPT_SENSOR)) {
		    sensor = opts[OPT_SENSOR].as<string > ();
		}

		if (opts.count(OPT_START_TIME) && !parseTime(_startTime, opts[OPT_START_TIME].as<string > ().c_str())) {
		    _error = true;
		    _errorMsg.assign("invalid start-time: format is: %Y[-%m[-%d[ %H[:%M[:%S]]]]]");
		    return;
		}

		if (opts.count(OPT_END_TIME) && !parseTime(_endTime, opts[OPT_END_TIME].as<string > ().c_str())) {
		    _error = true;
		    _errorMsg.assign("invalid end-time: format is: %Y[-%m[-%d[ %H[:%M[:%S]]]]]");
		    return;
		}

		if (_startTime >= _endTime) {
		    _errorMsg.assign("invalid time range: start-time >= end-time.");
		    _error = true;
		    return;
		}

		// required opts
		if (!opts.count(OPT_INPUT_FILE)) {
		    _errorMsg.assign(string(OPT_INPUT_FILE) + " not specified.");
		    _error = true;
		    return;
		}
		_inputFile = boost::filesystem::path(opts[OPT_INPUT_FILE].as<std::string > ());
		if (!boost::filesystem::exists(_inputFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputFile.file_string());
		    _errorMsg.append("\" does not exist.");
		    _error = true;
		    return;
		}
		if (!boost::filesystem::is_regular_file(_inputFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputFile.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}
	    }

	    bool NetflowViewerArguments::parseTime(TimeStamp &t, const std::string &str) {
		struct tm tmpTime;
		std::string format("%Y-%m-%d %H:%M:%S");

		time_t tmpTimet(0);
		gmtime_r(&tmpTimet, &tmpTime);

		char *ret;
		while (format.size() > 2 &&
			((ret = strptime(str.c_str(), format.c_str(), &tmpTime)) == NULL ||
			(*ret != '\0'))) {
		    format.resize(format.size() - 3);
		}
		if (ret == NULL || *ret != '\0') {
		    if ((ret = strptime(str.c_str(), format.c_str(), &tmpTime)) == NULL ||
			    (*ret != '\0')) {
			return false;
		    }
		}

		tmpTimet = timegm(&tmpTime);

		t.set(static_cast<uint32_t> (tmpTimet), 0);

		return true;
	    }
	}
    }
}
