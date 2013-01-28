#ifndef INFER_BIN_SEARCH_SEARCHCONFIGURATION_HPP_
#define INFER_BIN_SEARCH_SEARCHCONFIGURATION_HPP_

#include <string>
#include <tr1/unordered_set>

#include "SynappConfiguration.h"
#include "IPv4Network.hpp"

class SearchConfiguration : public SynappConfiguration {
  public:
	SearchConfiguration(const boost::filesystem::path &fileName)
		:SynappConfiguration(fileName),
		 _dataDirectory(),
		 _resultSchema(),
		 _dnsServers(),
		 _pgHost(),
		 _pgHostaddr(),
		 _pgPort(),
		 _pgDbname(),
		 _pgUser(),
		 _pgPassword(),
		 _pgConnectTimeout(),
		 _pgOptions(),
		 _pgSslmode(),
		 _pgRequiressl(),
		 _pgKrbsrvname(),
		 _pgGsslib(),
		 _pgService()
	{
		setOptionsDescriptions();
		parseOptions();
	}

	std::string dataDirectory() const {
		return _dataDirectory;
	}

	std::string resultSchema() const {
		return _resultSchema;
	}

	std::tr1::unordered_set <uint32_t> dnsServers() const {
		return _dnsServers;
	}

	std::string pgHost() const {
		return _pgHost;
	}

	std::string pgHostaddr() const {
		return _pgHostaddr;
	}

	std::string pgPort() const {
		return _pgPort;
	}

	std::string pgDbname() const {
		return _pgDbname;
	}

	std::string pgUser() const {
		return _pgUser;
	}

	std::string pgPassword() const {
		return _pgPassword;
	}

	std::string pgConnectTimeout() const {
		return _pgConnectTimeout;
	}

	std::string pgOptions() const {
		return _pgOptions;
	}

	std::string pgSslmode() const {
		return _pgSslmode;
	}

	std::string pgRequiressl() const {
		return _pgRequiressl;
	}

	std::string pgKrbsrvname() const {
		return _pgKrbsrvname;
	}

	std::string pgGsslib() const {
		return _pgGsslib;
	}

	std::string pgService() const {
		return _pgService;
	}


  private:
	void setOptionsDescriptions() {
		options.add_options()
			(
				"data-directory",
				boost::program_options::value <std::string>(),
				"the directory from which to obtain sensor data"
			)
			(
				"result-schema",
				boost::program_options::value <std::string>(),
				"the PostgreSQL schema in which to place the results table"
			)
			(
				"dns-server",
				boost::program_options::value <std::vector<IPv4Network> >(),
				"valid internal DNS servers"
			)
			(
				"psql-host",
				boost::program_options::value <std::string>(),
				"the PostgreSQL host connection parameter"
			)
			(
				"psql-hostaddr",
				boost::program_options::value <std::string>(),
				"the PostgreSQL hostaddr connection parameter"
			)
			(
				"psql-port",
				boost::program_options::value <std::string>(),
				"the PostgreSQL port connection parameter"
			)
			(
				"psql-dbname",
				boost::program_options::value <std::string>(),
				"the PostgreSQL dbname connection parameter"
			)
			(
				"psql-user",
				boost::program_options::value <std::string>(),
				"the PostgreSQL user connection parameter"
			)
			(
				"psql-password",
				boost::program_options::value <std::string>(),
				"the PostgreSQL password connection parameter"
			)
			(
				"psql-connect-timeout",
				boost::program_options::value <std::string>(),
				"the PostgreSQL connect-timeout connection parameter"
			)
			(
				"psql-options",
				boost::program_options::value <std::string>(),
				"the PostgreSQL options connection parameter"
			)
			(
				"psql-sslmode",
				boost::program_options::value <std::string>(),
				"the PostgreSQL sslmode connection parameter"
			)
			(
				"psql-requiressl",
				boost::program_options::value <std::string>(),
				"the PostgreSQL requiressl connection parameter"
			)
			(
				"psql-krbsrvname",
				boost::program_options::value <std::string>(),
				"the PostgreSQL krbsrvname connection parameter"
			)
			(
				"psql-gsslib",
				boost::program_options::value <std::string>(),
				"the PostgreSQL gsslib connection parameter"
			)
			(
				"psql-service",
				boost::program_options::value <std::string>(),
				"the PostgreSQL service connection parameter"
			)
		;
	}

	void parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
			return;
		}

		if (!vals.count("data-directory")) {
			_errorMsg.assign("data-directory not specified.");
			_error = true;
			return;
		}
		_dataDirectory = vals["data-directory"].as<std::string>();
		if (_dataDirectory.empty()) {
			_errorMsg.assign("data-directory cannot be empty.");
			_error = true;
			return;
		}

		if (!vals.count("result-schema")) {
			_errorMsg.assign("result-schema not specified.");
			_error = true;
			return;
		}
		_resultSchema = vals["result-schema"].as<std::string>();
		if (_resultSchema.empty()) {
			_errorMsg.assign("result-schema cannot be empty.");
			_error = true;
			return;
		}
		if (vals.count("dns-server")) {
			const std::vector<IPv4Network> &dnsServers(vals["dns-server"].as<std::vector<IPv4Network> >());
			for (std::vector<IPv4Network>::const_iterator net(dnsServers.begin());
				 net != dnsServers.end();
				 ++net)
			{
				if (net->netmask() != std::numeric_limits<uint32_t>::max()) {
					_errorMsg.assign("dns-server must be an ip address");
					_error = true;
					return;
				}
				_dnsServers.insert(net->rawNetwork());
			}
		}

		if (vals.count("psql-host")) {
			_pgHost = vals["psql-host"].as<std::string>();
			if (_pgHost.empty()) {
				_errorMsg.assign("psql-host cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-hostaddr")) {
			_pgHostaddr = vals["psql-hostaddr"].as<std::string>();
			if (_pgHostaddr.empty()) {
				_errorMsg.assign("psql-hostaddr cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-port")) {
			_pgPort = vals["psql-port"].as<std::string>();
			if (_pgPort.empty()) {
				_errorMsg.assign("psql-port cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-dbname")) {
			_pgDbname = vals["psql-dbname"].as<std::string>();
			if (_pgDbname.empty()) {
				_errorMsg.assign("psql-dbname cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-user")) {
			_pgUser = vals["psql-user"].as<std::string>();
			if (_pgUser.empty()) {
				_errorMsg.assign("psql-user cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-password")) {
			_pgPassword = vals["psql-password"].as<std::string>();
			if (_pgPassword.empty()) {
				_errorMsg.assign("psql-password cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-connect-timeout")) {
			_pgConnectTimeout = vals["psql-connect-timeout"].as<std::string>();
			if (_pgConnectTimeout.empty()) {
				_errorMsg.assign("psql-connect-timeout cannot be empty if "
								 "specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-options")) {
			_pgOptions = vals["psql-options"].as<std::string>();
			if (_pgOptions.empty()) {
				_errorMsg.assign("psql-options cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-sslmode")) {
			_pgSslmode = vals["psql-sslmode"].as<std::string>();
			if (_pgSslmode.empty()) {
				_errorMsg.assign("psql-sslmode cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-requiressl")) {
			_pgRequiressl = vals["psql-requiressl"].as<std::string>();
			if (_pgRequiressl.empty()) {
				_errorMsg.assign("psql-requiressl cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-krbsrvname")) {
			_pgKrbsrvname = vals["psql-krbsrvname"].as<std::string>();
			if (_pgKrbsrvname.empty()) {
				_errorMsg.assign("psql-krbsrvname cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-gsslib")) {
			_pgGsslib = vals["psql-gsslib"].as<std::string>();
			if (_pgGsslib.empty()) {
				_errorMsg.assign("psql-gsslib cannot be empty if specified.");
				_error = true;
				return;
			}
		}
		if (vals.count("psql-service")) {
			_pgService = vals["psql-service"].as<std::string>();
			if (_pgService.empty()) {
				_errorMsg.assign("psql-service cannot be empty if specified.");
				_error = true;
				return;
			}
		}
	}

	std::string _dataDirectory;
	std::string _resultSchema;
	std::tr1::unordered_set<uint32_t> _dnsServers;

	std::string _pgHost;
	std::string _pgHostaddr;
	std::string _pgPort;
	std::string _pgDbname;
	std::string _pgUser;
	std::string _pgPassword;
	std::string _pgConnectTimeout;
	std::string _pgOptions;
	std::string _pgSslmode;
	std::string _pgRequiressl;
	std::string _pgKrbsrvname;
	std::string _pgGsslib;
	std::string _pgService;
};

#endif
