#ifndef POSTGRESQL_CONFIGURATION_LOADER_H
#define POSTGRESQL_CONFIGURATION_LOADER_H

#include "configurationLoader.hpp"
#include "postgreSQL.h"

class PostgreSQLConfigurationLoader : public ConfigurationLoader {
  public:
	PostgreSQLConfigurationLoader(PGconn *, const std::string, const std::string);
	PostgreSQLConfigurationLoader();
	void init(PGconn *, const std::string, const std::string);
	bool load(std::map <std::string, std::vector <std::string> > &options);
	bool operator!() const;
	std::string error() const;
  private:
	PGconn *postgreSQL;
	std::string schema;
	std::string table;
	bool _error;
	std::string errorMessage;
};

#endif
