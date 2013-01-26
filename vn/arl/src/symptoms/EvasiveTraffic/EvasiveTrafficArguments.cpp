/* 
 * File:   EvasiveTrafficArguments.cpp
 * Author: Mike
 * 
 * Created on December 2, 2009, 10:27 PM
 */

#include "EvasiveTrafficArguments.h"
#include "EvasiveTrafficParams.h"

#include <iostream>

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;

	    EvasiveTrafficArguments::EvasiveTrafficArguments(int argc, char** argv)
	    : SynappArguments(argc, argv),
	    _configFile("/usr/local/etc/symtoms.conf"),
	    _help(false),
	    ttl(DEFAULT_TTL_VALUE),
	    _stdout(true),
            sensor("<sensor>") {
		setOptionsDescriptions();
		parseOptions();
	    }

	    void EvasiveTrafficArguments::setOptionsDescriptions() {
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
			MAKE_CMD(OPT_TTL, OPT_SHORT_TTL),
			boost::program_options::value <int> (),
			"Specifies the TTL threshold. 10 is default."
			)
			(
			MAKE_CMD(OPT_STDOUT, OPT_SHORT_STDOUT),
			"do NOT output to stdout. Default on."
			)
                        (
			MAKE_CMD(OPT_SENSOR, OPT_SHORT_SENSOR),
			boost::program_options::value <string> (),
			"Specify the sensor name, optional but recommended as NetFlow file doesn't contain this name."
			);

		arguments.add(reqArgs);
		arguments.add(optArgs);
	    }

	    const boost::filesystem::path & EvasiveTrafficArguments::inputNetFlowFile() const {
		return _inputDir;
	    }

	    const boost::filesystem::path & EvasiveTrafficArguments::configFile() const {
		return _configFile;
	    }

	    const boost::filesystem::path& EvasiveTrafficArguments::outputFile() const {
		return _output;
	    }

	    bool EvasiveTrafficArguments::isStdOut() const {
		return _stdout;
	    }

	    bool EvasiveTrafficArguments::isHelp() const {
		return _help;
	    }

	    const int EvasiveTrafficArguments::getTTL() const {
		return ttl;
	    }

            const std::string EvasiveTrafficArguments::getSensorName() const {
                return sensor;
            }

	    void EvasiveTrafficArguments::parseOptions() {
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

		    EvasiveTrafficConfiguration* config = new EvasiveTrafficConfiguration(_configFile);

		    if(!config) {
			_errorMsg.assign("Error in the configuration file (");
			_errorMsg.append(_configFile.file_string());
			_errorMsg.append("): ");
			_errorMsg.append(config->error());
			_error = true;

			delete config;
			
			return;
		    }

		    ttl = config->getTTL();

		    delete config;
		}

		if(opts.count(OPT_TTL)) {
		    ttl = opts[OPT_TTL].as<int>();
		}

                if (opts.count(OPT_SENSOR)) {
                    sensor = opts[OPT_SENSOR].as<std::string> ();
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
		    std::cerr << "WARN: File \"" << _output.file_string()
		         << "\" already exists. Data will be appended." << std::endl;
		}
	    }
	}
    }
}