#include "configuration.h"

Configuration::Configuration(ConfigurationLoader &loader) {
	open(loader);
}

Configuration::Configuration() 
	:_error(true)
{
}

bool Configuration::open(ConfigurationLoader &loader) {
	if (!loader.load(options)) {
		_error = true;
		errorMessage = loader.error();
	} else {
		_error = false;
	}

	return !_error;
}

bool Configuration::operator!() const {
	return _error;
}

std::string Configuration::error() const {
	return errorMessage;
}

std::string Configuration::getString(const std::string option) {
	if (options[option].size()) {
		return options[option][0];
	}
	return "";
}

std::vector <std::string> &Configuration::getStrings(const std::string option) {
	return options[option];
}

size_t Configuration::getNumber(const std::string option) {
	if (options[option].size()) {
		return strtoul(options[option][0].c_str(), NULL, 10);
	}
	return std::numeric_limits <size_t>::max();
}

void printError(const std::string program, const std::string reason) {
  std::cerr << program << ": " << reason << std::endl;
}

void printError(const std::string program, const std::string object,
                const std::string reason) {
  std::cerr << program << ": " << object << ": " << reason << std::endl;
}

void printError(const std::string program, const std::string function,
                const std::string object, const std::string reason) {
  std::cerr << program << ": " << function << ": " << object << ": " << reason
            << std::endl;
}

std::string makePGConnectionString(Configuration &conf) {
  std::string connectionString;
  if (!conf.getString("postgreSQLHost").empty()) {
    connectionString = "host = " +
                        conf.getString("postgreSQLHost") + ' ';
  }
  if (conf.getNumber("postgreSQLPort") != 0) {
    connectionString = " port = " +
                        conf.getString("postgreSQLPort") + ' ';
  }
  connectionString += "dbname = " +
                      conf.getString("postgreSQLDatabase") +
                      " user = " +
                      conf.getString("postgreSQLUser") +
                      " password = " +
                      conf.getString("postgreSQLPassword") +
                      " connect_timeout = " +
                      conf.getString("postgreSQLTimeout");
  return connectionString;
}
