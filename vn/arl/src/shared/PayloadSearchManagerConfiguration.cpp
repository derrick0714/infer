#include "PayloadSearchManagerConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

PayloadSearchManagerConfiguration::
PayloadSearchManagerConfiguration(const fs::path &fileName)
	:SynappConfiguration(fileName),
	 _maxMTU(0),
	 _maxFlows(0),
	 _threadCount(0),
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
	 _pgService(),
	 _pgSchema()
{
	setOptionsDescriptions();
	parseOptions();
}

void PayloadSearchManagerConfiguration::setOptionsDescriptions() {
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
		(
			"result-schema",
			boost::program_options::value <std::string>(),
			"the PostgreSQL schema in which to place the results table"
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

void PayloadSearchManagerConfiguration::parseOptions() {
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

	if (!vals.count("result-schema")) {
		_errorMsg.assign("result-schema not specified.");
		_error = true;
		return;
	}
	_pgSchema = vals["result-schema"].as<std::string>();
	if (_pgSchema.empty()) {
		_errorMsg.assign("result-schema cannot be empty.");
		_error = true;
		return;
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

uint16_t PayloadSearchManagerConfiguration::maxMTU() const {
	return _maxMTU;
}

size_t PayloadSearchManagerConfiguration::maxFlows() const {
	return _maxFlows;
}

size_t PayloadSearchManagerConfiguration::threadCount() const {
	return _threadCount;
}

std::string PayloadSearchManagerConfiguration::pgHost() const {
	return _pgHost;
}

std::string PayloadSearchManagerConfiguration::pgHostaddr() const {
	return _pgHostaddr;
}

std::string PayloadSearchManagerConfiguration::pgPort() const {
	return _pgPort;
}

std::string PayloadSearchManagerConfiguration::pgDbname() const {
	return _pgDbname;
}

std::string PayloadSearchManagerConfiguration::pgUser() const {
	return _pgUser;
}

std::string PayloadSearchManagerConfiguration::pgPassword() const {
	return _pgPassword;
}

std::string PayloadSearchManagerConfiguration::pgConnectTimeout() const {
	return _pgConnectTimeout;
}

std::string PayloadSearchManagerConfiguration::pgOptions() const {
	return _pgOptions;
}

std::string PayloadSearchManagerConfiguration::pgSslmode() const {
	return _pgSslmode;
}

std::string PayloadSearchManagerConfiguration::pgRequiressl() const {
	return _pgRequiressl;
}

std::string PayloadSearchManagerConfiguration::pgKrbsrvname() const {
	return _pgKrbsrvname;
}

std::string PayloadSearchManagerConfiguration::pgGsslib() const {
	return _pgGsslib;
}

std::string PayloadSearchManagerConfiguration::pgService() const {
	return _pgService;
}

std::string PayloadSearchManagerConfiguration::pgSchema() const {
	return _pgSchema;
}

} // namespace vn
} // namespace arl
} // namespace shared
