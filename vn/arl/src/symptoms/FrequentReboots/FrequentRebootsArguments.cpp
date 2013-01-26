/* 
 * File:   FrequentRebootsArguments.cpp
 * Author: Mike
 * 
 * Created on December 2, 2009, 10:27 PM
 */

#include "FrequentRebootsArguments.h"
#include "FrequentRebootsParams.h"
#include "HIEAppStore.hpp"
#include "../SymptomDefaults.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    FrequentRebootsArguments::FrequentRebootsArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
	    _configFile(CONFIG_FILE),
	    _help(false),
	    _stdout(true),
	    sensor(SENSOR_NAME),
            eventsAtReboot(DEFAULT_EVENTS_AT_REBOOT),
            bootTimeWindow(DEFAULT_BOOT_TIME_WINDOW),
            rebootCount(DEFAULT_REBOOT_COUNT),
            watchTime(DEFAULT_WATCH_TIME),
            _hostAppMap(string(DEFAULT_STATE_DIR).append("app_map")),
            _hostRebootMap(string(DEFAULT_STATE_DIR).append("reboot_map")),
            appStore(NULL) {
		setOptionsDescriptions();
		parseOptions();
	    }

            FrequentRebootsArguments::~FrequentRebootsArguments() {
                if(appStore != NULL) {
                    delete appStore;
                }
            }

	    void FrequentRebootsArguments::setOptionsDescriptions() {
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
			MAKE_CMD(OPT_INPUT_PCAP, OPT_SHORT_INPUT_PCAP),
			value <string> (),
			"pcap input file"
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
			MAKE_CMD(OPT_STATE_DIR, OPT_SHORT_STATE_DIR),
			boost::program_options::value <string> (),
			"Specify the directory with state files from previous execution or an empty directory."
			)
                        (
                        MAKE_CMD(OPT_APP_DEF, OPT_SHORT_APP_DEF), 
                        boost::program_options::value<std::string> (),
                        "Application definition file"
                        )
			(
			MAKE_CMD(OPT_SENSOR, OPT_SHORT_SENSOR),
			boost::program_options::value <string> (),
			"Specify the sensor name, optional but recommended as NetFlow file doesn't contain this name."
			)
                        (
                        MAKE_CMD(OPT_EVENTS_AT_REBOOT, OPT_SHORT_EVENTS_AT_REBOOT),
                        boost::program_options::value <int> (),
			"How many events constitute a reboot."
			)
                        (
                        MAKE_CMD(OPT_BOOT_TIME_WINDOW, OPT_SHORT_BOOT_TIME_WINDOW),
                        boost::program_options::value <int> (),
			"At what time window should the events happen to declare a reboot. Defined in seconds. Default: 300"
			)
                        (
                        MAKE_CMD(OPT_WATCH_TIME, OPT_SHORT_WATCH_TIME),
                        boost::program_options::value <int> (),
			"At what time window should the rebootCount events happen. Defined in hours. Default: 24"
			)
                        (
                        MAKE_CMD(OPT_REBOOT_COUNT, OPT_SHORT_REBOOT_COUNT),
                        boost::program_options::value <int> (),
                        "How many reboots should there be before we flag the host."
			);

		arguments.add(reqArgs);
		arguments.add(optArgs);
	    }

            const boost::filesystem::path& FrequentRebootsArguments::inputPcapFile() const {
                return _inputPcap;
            }

	    const boost::filesystem::path & FrequentRebootsArguments::inputNetFlowFile() const {
		return _inputDir;
	    }

	    const boost::filesystem::path & FrequentRebootsArguments::configFile() const {
		return _configFile;
	    }

	    const boost::filesystem::path& FrequentRebootsArguments::outputFile() const {
		return _output;
	    }

            int FrequentRebootsArguments::getEventsAtReboot() const {
                return eventsAtReboot;
            }

            int FrequentRebootsArguments::getBootTimeWindow() const {
                return bootTimeWindow;
            }

            int FrequentRebootsArguments::getRebootCount() const {
                return rebootCount;
            }

            int FrequentRebootsArguments::getWatchTime() const {
                return watchTime;
            }

            HIEAppStore& FrequentRebootsArguments::getApplicationStore() const {
                return *appStore;
            }

	    const boost::filesystem::path& FrequentRebootsArguments::getHostAppMapFile() const {
		return _hostAppMap;
	    }

	    const boost::filesystem::path& FrequentRebootsArguments::getHostRebootFile() const {
		return _hostRebootMap;
	    }

            const string FrequentRebootsArguments::getSensorName() const {
		return sensor;
	    }

	    bool FrequentRebootsArguments::isStdOut() const {
		return _stdout;
	    }

	    bool FrequentRebootsArguments::isHelp() const {
		return _help;
	    }

            bool FrequentRebootsArguments::hasPcap() const {
                return _hasPcap;
            }

	    void FrequentRebootsArguments::parseOptions() {
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

                boost::filesystem::path dnsFile;

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

		    FrequentRebootsConfiguration* config = new FrequentRebootsConfiguration(_configFile);

		    if(!config) {
			_errorMsg.assign("Error in the configuration file (");
			_errorMsg.append(_configFile.file_string());
			_errorMsg.append("): ");
			_errorMsg.append(config->error());
			_error = true;

			delete config;
			
			return;
		    }

                    bootTimeWindow = config->getBootTimeWindow();
                    eventsAtReboot = config->getEventsAtReboot();
                    rebootCount = config->getRebootCount();
                    watchTime = config->getWatchTime();

                    string dir = config->getStateDirectory().filename();
                    _hostAppMap = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("app_map"));
                    _hostRebootMap = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("reboot_map"));
                    dnsFile = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("dns_map"));

		    delete config;
		}

                if (opts.count(OPT_STATE_DIR)) {
		    string dir = opts[OPT_STATE_DIR].as<std::string> ();

                    if(!boost::filesystem::exists(boost::filesystem::path(dir))) {
                        _errorMsg.append("\"");
                        _errorMsg.append(dir);
                        _errorMsg.append("\" does not exist.");
                        _error = true;
                        return;
                    }

                    _hostAppMap = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("app_map"));
                    _hostRebootMap = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("reboot_map"));
                    dnsFile = boost::filesystem::path(string(dir).append(boost::filesystem::path::string_type(1,boost::filesystem::slash<boost::filesystem::path>::value)).append("dns_map"));
		}

		if (opts.count(OPT_APP_DEF)) {
                    boost::filesystem::path appFile = boost::filesystem::path(opts[OPT_APP_DEF].as<std::string > ());

                    if(!boost::filesystem::exists(appFile)) {
                        _errorMsg.assign("Application definition file does not exist: ");
                        _errorMsg.append(appFile.file_string());
                        _error = true;
                        return;
                    }

                    appStore = new HIEAppStore(appFile, dnsFile);

                    if(appStore->errored()) {
                        _errorMsg.assign(appStore->error());
                        _error = true;
                        return;
                    }
		}

                if(appStore == NULL || appStore->size() == 0) {
                    _errorMsg.assign("Application definitions not found");
                    _error = true;
                    return;
                }

                if (opts.count(OPT_EVENTS_AT_REBOOT)) {
		    eventsAtReboot = opts[OPT_EVENTS_AT_REBOOT].as<int> ();
		}

                if (opts.count(OPT_BOOT_TIME_WINDOW)) {
		    bootTimeWindow = opts[OPT_BOOT_TIME_WINDOW].as<int> ();
		}

                if (opts.count(OPT_WATCH_TIME)) {
		    watchTime = opts[OPT_WATCH_TIME].as<int> ();
		}

                if (opts.count(OPT_REBOOT_COUNT)) {
		    rebootCount = opts[OPT_REBOOT_COUNT].as<int> ();
		}

		if (boost::filesystem::exists(_hostAppMap) && !boost::filesystem::is_regular_file(_hostAppMap)) {
		    _errorMsg.assign("invalid state file: \"");
		    _errorMsg.append(_hostAppMap.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

		if (boost::filesystem::exists(_hostRebootMap) && !boost::filesystem::is_regular_file(_hostRebootMap)) {
		    _errorMsg.assign("invalid state file: \"");
		    _errorMsg.append(_hostRebootMap.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

                _hasPcap = false;
                if(opts.count(OPT_INPUT_PCAP)) {
                    _inputPcap = boost::filesystem::path(opts[OPT_INPUT_PCAP].as<std::string > ());
                    if (boost::filesystem::exists(_inputPcap) && !boost::filesystem::is_regular_file(_inputPcap)) {
                        _errorMsg.assign("invalid state file: \"");
                        _errorMsg.append(_inputPcap.file_string());
                        _errorMsg.append("\" is not a regular file.");
                        _error = true;
                        return;
                    } else {
                        _hasPcap = true;
                    }
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