/* 
 * File:   EvasiveTrafficArguments.cpp
 * Author: Mike
 * 
 * Created on December 2, 2009, 10:27 PM
 */

#include "DNSFreeFlowsArguments.h"
#include "DNSFreeFlowsParams.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    DNSFreeFlowsArguments::DNSFreeFlowsArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
	    _configFile("/usr/local/etc/symptoms.conf"),
            dnsTimeOut(24),
	    _help(false) {
		setOptionsDescriptions();
		parseOptions();
	    }

	    void DNSFreeFlowsArguments::setOptionsDescriptions() {
		using namespace boost::program_options;

		options_description reqArgs("Symptom Required Arguments");
		options_description optArgs("Symptom Optional Arguments");

		reqArgs.add_options()
			(
			MAKE_CMD(OPT_INPUT_NETFLOW, OPT_SHORT_INPUT_NETFLOW),
			value <std::string > (),
			"Netflow input file"
			)
                        (
			MAKE_CMD(OPT_INPUT_PCAP, OPT_SHORT_INPUT_PCAP),
			value <std::string > (),
			"pcap input file"
			)
                        (
			MAKE_CMD(OPT_OUTFILE, OPT_SHORT_OUTFILE),
			value<std::string>(),
			"Output file"
			);

		optArgs.add_options()
			(
			MAKE_CMD(OPT_CONFIG_FILE, OPT_SHORT_CONFIG_FILE),
			boost::program_options::value <std::string > (),
			"file from which to read additional configuration options."
			)
			(
			MAKE_CMD(OPT_HELP, OPT_SHORT_HELP),
			"prints a help message to stdout"
			)
			(
			MAKE_CMD(OPT_DNS_TIMEOUT, OPT_SHORT_DNS_TIMEOUT),
			boost::program_options::value <int> (),
			"Specifies the DNS timeout. 24 hours is default."
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

	    const boost::filesystem::path & DNSFreeFlowsArguments::inputNetFlowFile() const {
		return _inputNetFile;
	    }

            const boost::filesystem::path & DNSFreeFlowsArguments::inputPcapFile() const {
                return _inputPcapFile;
            }

	    const boost::filesystem::path & DNSFreeFlowsArguments::configFile() const {
		return _configFile;
	    }

            const boost::filesystem::path& DNSFreeFlowsArguments::outputFile() const {
                return _output;
            }

            const boost::filesystem::path& DNSFreeFlowsArguments::getStateFile() const {
                return _stateFile;
            }

            const string DNSFreeFlowsArguments::getSensorName() const{
                return sensor;
            }

            const int DNSFreeFlowsArguments::getDnsTimeOut() const {
                return dnsTimeOut;
            }

            bool DNSFreeFlowsArguments::help() const {
		return _help;
	    }

            bool DNSFreeFlowsArguments::isStdOut() const {
		return _stdout;
	    }

            void DNSFreeFlowsArguments::parseOptions() {
		boost::program_options::variables_map opts;
		if (!_parseOptions(opts)) {
		    return;
		}

                // optional options
		if (opts.count(OPT_HELP)) {
		    _help = true;
		    return;
		}

                if(opts.count(OPT_STDOUT)) {
		    _stdout = false;
		}

                if (opts.count(OPT_CONFIG_FILE)) {
                    _configFile = boost::filesystem::path(opts[OPT_CONFIG_FILE].as<std::string > ());

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

		    DNSFreeFlowsConfiguration* config = new DNSFreeFlowsConfiguration(_configFile);

		    if(!config) {
			_errorMsg.assign("Error in the configuration file (");
			_errorMsg.append(_configFile.file_string());
			_errorMsg.append("): ");
			_errorMsg.append(config->error());
			_error = true;

			delete config;
			
			return;
		    }

                    dnsTimeOut = config->getTimeOut();
                    _stateFile = config->getStateFile();

		    delete config;
		}

                if (opts.count(OPT_DNS_TIMEOUT)) {
                    dnsTimeOut = opts[OPT_DNS_TIMEOUT].as<int> ();
                }

                if (opts.count(OPT_SENSOR)) {
                    sensor = opts[OPT_SENSOR].as<std::string> ();
                }

		// required opts
                if (!opts.count(OPT_STATE_FILE)) {
		    _errorMsg.assign(string(OPT_STATE_FILE) + " not specified.");
		    _error = true;
		    return;
		}
		_stateFile = boost::filesystem::path(opts[OPT_STATE_FILE].as<std::string > ());
		if (boost::filesystem::exists(_stateFile) && !boost::filesystem::is_regular_file(_stateFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_stateFile.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

                if (!opts.count(OPT_INPUT_NETFLOW)) {
		    _errorMsg.assign(string(OPT_INPUT_NETFLOW) + " not specified.");
		    _error = true;
		    return;
		}
		_inputNetFile = boost::filesystem::path(opts[OPT_INPUT_NETFLOW].as<std::string > ());
		if (!boost::filesystem::exists(_inputNetFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputNetFile.file_string());
		    _errorMsg.append("\" does not exist.");
		    _error = true;
		    return;
		}
		if (!boost::filesystem::is_regular_file(_inputNetFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputNetFile.file_string());
		    _errorMsg.append("\" is not a regular file.");
		    _error = true;
		    return;
		}

                if (!opts.count(OPT_INPUT_PCAP)) {
		    _errorMsg.assign(string(OPT_INPUT_PCAP) + " not specified.");
		    _error = true;
		    return;
		}
		_inputPcapFile = boost::filesystem::path(opts[OPT_INPUT_PCAP].as<std::string > ());
		if (!boost::filesystem::exists(_inputPcapFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputPcapFile.file_string());
		    _errorMsg.append("\" does not exist.");
		    _error = true;
		    return;
		}
		if (!boost::filesystem::is_regular_file(_inputPcapFile)) {
		    _errorMsg.append("\"");
		    _errorMsg.append(_inputPcapFile.file_string());
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
		    std::cerr << "WARN: File \"" << _output.file_string()
		         << "\" already exists. Data will be appended." << endl;
		}
	    }
	}
    }
}