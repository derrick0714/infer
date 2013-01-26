/* 
 * File:   PeerToPeerArguments.cpp
 * Author: Mike
 * 
 * Created on December 2, 2009, 10:27 PM
 */

#include <boost/lexical_cast.hpp>

#include "PeerToPeerArguments.h"
#include "PeerToPeerParams.h"
#include "../SymptomDefaults.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    PeerToPeerArguments::PeerToPeerArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
                    _configFile(CONFIG_FILE),
                    _help(false),
                    _phase2(true),
                    _stdout(true),
                    sensor(SENSOR_NAME),
                    _startTime(0, 0), _endTime(0xFFFFFFFF, 0xFFFFFFFF),
                    _ephPortStart(1024), _ephPortEnd(65536)
	    {
		setOptionsDescriptions();
		parseOptions();
	    }

	    void PeerToPeerArguments::setOptionsDescriptions() {
		using namespace boost::program_options;

		options_description reqArgs("Symptom Required Arguments");
		options_description optArgs("Symptom Optional Arguments");

		reqArgs.add_options()
			(
			MAKE_CMD(OPT_INPUT_DNSFREE, OPT_SHORT_INPUT_DNSFREE),
			value <string> (),
			"DNSFreeFlows symptom's output file (Input for this symptom)"
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
                        MAKE_CMD(OPT_PHASE2, OPT_SHORT_PHASE2),
                        "Disable phase 2 processing"
                        )
			(
			MAKE_CMD(OPT_STDOUT, OPT_SHORT_STDOUT),
			"do NOT output to stdout. Default on."
			)
			(
			MAKE_CMD(OPT_ZONE, OPT_SHORT_ZONE),
			boost::program_options::value <string> (),
			"Specify the monitored zone. ex: 192.168.1.2/24 or 2001:480:60:55::3/96"
			)
			(
			MAKE_CMD(OPT_SENSOR, OPT_SHORT_SENSOR),
			boost::program_options::value <string> (),
			"Specify the sensor name, optional but recommended as NetFlow file doesn't contain this name."
			)
                        (
			MAKE_CMD(OPT_START_TIME, OPT_SHORT_START_TIME),
			boost::program_options::value <string> (),
			"Start time. format is: %Y[-%m[-%d[ %H[:%M[:%S]]]]]."
			)
                        (
			MAKE_CMD(OPT_END_TIME, OPT_SHORT_END_TIME),
			boost::program_options::value <string> (),
			"End time. format is: %Y[-%m[-%d[ %H[:%M[:%S]]]]]."
			)
                        (
                        MAKE_CMD(OPT_EPH_PORT, OPT_SHORT_EPH_PORT),
                        boost::program_options::value <string> (),
                        "Ephemeral port definition, [start port]-[end port]. Default (1024 to 65536)"
                        );

		arguments.add(reqArgs);
		arguments.add(optArgs);
	    }

	    const boost::filesystem::path & PeerToPeerArguments::inputDNSFreeFile() const {
		return _inputFile;
	    }

	    const boost::filesystem::path & PeerToPeerArguments::configFile() const {
		return _configFile;
	    }

	    const boost::filesystem::path& PeerToPeerArguments::outputFile() const {
		return _output;
	    }

	    const IPZone& PeerToPeerArguments::getMonitoredZone() const {
		return zone;
	    }

            const string PeerToPeerArguments::getSensorName() const {
		return sensor;
	    }

            const TimeStamp& PeerToPeerArguments::getStartTime() const {
		return _startTime;
	    }

	    const TimeStamp& PeerToPeerArguments::getEndTime() const {
		return _endTime;
	    }

	    bool PeerToPeerArguments::isStdOut() const {
		return _stdout;
	    }

	    bool PeerToPeerArguments::isHelp() const {
		return _help;
	    }

            bool PeerToPeerArguments::isPhase2() const {
                return _phase2;
            }

            int PeerToPeerArguments::getEphemeralPortStart() const {
                return _ephPortStart;
            }

            int PeerToPeerArguments::getEphemeralPortEnd() const {
                return _ephPortEnd;
            }


	    void PeerToPeerArguments::parseOptions() {
		boost::program_options::variables_map opts;
		if (!_parseOptions(opts)) {
		    return;
		}

		// optional options
		if (opts.count(OPT_HELP) || opts.size() == 0) {
		    _help = true;
		    return;
		}

                if(opts.count(OPT_PHASE2)) {
                    _phase2 = false;
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

		    PeerToPeerConfiguration* config = new PeerToPeerConfiguration(_configFile);

		    if(!config) {
			_errorMsg.assign("Error in the configuration file (");
			_errorMsg.append(_configFile.file_string());
			_errorMsg.append("): ");
			_errorMsg.append(config->error());
			_error = true;

			delete config;
			
			return;
		    }

		    zone = config->getMonitoredZone();
                    _phase2 = config->isPhaseTwo();
                    _ephPortStart = config->getEphemeralPortStart();
                    _ephPortEnd = config->getEphemeralPortEnd();

		    delete config;
		}

                if(opts.count(OPT_EPH_PORT)) {
                    string port_str = opts[OPT_EPH_PORT].as<string>();
                    string::size_type dash = port_str.find("-");
                    string port1_str = port_str.substr(0, dash);
                    string port2_str = port_str.substr(dash + 1);

                    try {
                        _ephPortStart = boost::lexical_cast< int >(port1_str);
                        _ephPortEnd = boost::lexical_cast< int >(port2_str);
                    } catch( const boost::bad_lexical_cast & ) {
			_errorMsg.assign("Error parsing numbers: ").append(port_str);
			_error = true;
			return;
                    }
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

		if(zone.getEntryCount() == 0) {
		    cerr << "# monitired zone not speficied. All hosts will be included." << endl;
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
		if (!opts.count(OPT_INPUT_DNSFREE)) {
		    _errorMsg.assign(string(OPT_INPUT_DNSFREE) + " not specified.");
		    _error = true;
		    return;
		}
		_inputFile = boost::filesystem::path(opts[OPT_INPUT_DNSFREE].as<std::string > ());
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

	    bool PeerToPeerArguments::parseTime(TimeStamp &t, const std::string &str) {
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
