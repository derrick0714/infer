#include "postgreSQLConfigurationLoader.h"

PostgreSQLConfigurationLoader::PostgreSQLConfigurationLoader
		(PGconn *postgreSQL, const std::string schema, const std::string table)
	:postgreSQL(postgreSQL), schema(schema), table(table), _error(false)
{
}

PostgreSQLConfigurationLoader::PostgreSQLConfigurationLoader()
	:_error(true), errorMessage("PostgreSQLConfigurationLoader: not initialized.")
{
}

void PostgreSQLConfigurationLoader::init(PGconn *postgreSQL, 
										 const std::string schema,
										 const std::string table)
{
	this -> postgreSQL = postgreSQL;
	this -> schema = schema;
	this -> table = table;
	_error = false;
}

bool PostgreSQLConfigurationLoader::load
		(std::map <std::string, std::vector <std::string> > &options)
{
	std::string pgQuery;
	
	pgQuery.assign("SELECT \"name\", \"value\", \"default\" FROM \"");
	pgQuery.append(schema + "\".\"");
	pgQuery.append(table + "\"");

	PGresult *result;

	result = PQexecParams(postgreSQL, pgQuery.c_str(), 0, 
						  NULL, NULL, NULL, NULL, 1);
	
	_error = false;
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		_error = true;
		errorMessage.assign(PQerrorMessage(postgreSQL));
		return false;
	}

	std::string option, value;
	for (int i = 0; i < PQntuples(result); ++i) {
		option.assign(PQgetvalue(result, i, 0));
		if (!PQgetisnull(result, i, 1)) {
			value.assign(PQgetvalue(result, i, 1));
		} else {
			value.assign(PQgetvalue(result, i, 2));
		}
		options[option].push_back(value);
	}
	
	PQclear(result);
	return true;
}

bool PostgreSQLConfigurationLoader::operator!() const {
  return _error;
}

std::string PostgreSQLConfigurationLoader::error() const {
  return errorMessage;
}
