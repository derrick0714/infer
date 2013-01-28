#include <iostream> 
#include <sstream>
#include <vector>
#include <map>
#include <tr1/unordered_map>
#include <netinet/in.h>
#include <sys/param.h>
#include <fetch.h>

#include "configuration.hpp"
#include "stringHelpers.h"
#include "address.h"
#include "postgreSQL.h"
#include "clock.hpp"
#include "PostgreSQLConnection.hpp"

#include "country.h"

#define COUNTRY_MAP_TABLE_SCHEMA "\"countryNumber\" SMALLINT NOT NULL, \
								\"firstIP\" uint32 NOT NULL, \
								\"lastIP\" uint32 NOT NULL, \
								PRIMARY KEY (\"firstIP\", \"lastIP\")"

using namespace std;
using namespace tr1;

void cidrToRange(pair <uint32_t, uint32_t> &range, const std::string &network)
{
	static size_t slash = network.rfind('/');
	range.first = textToIP(network.substr(0, slash));
	range.second = range.first +
					pow((double)2,
						(double)(32 - strtoul(network.substr(slash + 1).c_str(),
											  NULL, 10))) - 1;
}

bool getline(char &ch, string &line, FILE *file) {
	line.clear();
	while ((ch = getc(file)) != '\n' && ch != EOF) {
		line += ch;
	}
	if (ch != EOF) {
		return true;
	}
	return false;
}

void updateCountryIPBlocks(map <uint32_t, Country> &countryIPBlocks,
						   uint32_t &firstIP, uint32_t &lastIP,
						   int16_t &countryNumber)
{
	static Country country;
	static map <uint32_t, Country>::iterator newItr, adjacentItr;
	static bool update = false;
	country.lastIP = lastIP;
	country.countryNumber = countryNumber;
	newItr = countryIPBlocks.insert(make_pair(firstIP, country)).first;
	--(adjacentItr = newItr);
	if (country.countryNumber == adjacentItr -> second.countryNumber &&
		firstIP == adjacentItr -> second.lastIP + 1 && adjacentItr != newItr)
	{ 
		update = true;
		firstIP = adjacentItr -> first;
		countryIPBlocks.erase(adjacentItr);
	}
	++(adjacentItr = newItr);
	if (country.countryNumber == adjacentItr -> second.countryNumber &&
		lastIP == adjacentItr -> first - 1 && adjacentItr != newItr)
	{
		update = true;
		country.lastIP = adjacentItr -> second.lastIP;
		countryIPBlocks.erase(adjacentItr);
	}
	if (update) {
		countryIPBlocks.erase(newItr);
		countryIPBlocks.insert(make_pair(firstIP, country));
	}
}

bool commitCountryIPBlocks(PGconn *postgreSQL, size_t flushSize,
						   const map <uint32_t, Country> &countryIPBlocks)
{
	Clock clock("Inserted", "rows");
	PGBulkInserter pgBulkInserter(postgreSQL, "Maps", "countryIPBlocks",
								  flushSize, "%d, %ud, %ud");
	if (!preparePGTable(postgreSQL, "Maps", "countryIPBlocks",
						COUNTRY_MAP_TABLE_SCHEMA))
	{
		return false;
	}
	clock.start();
	for (map<uint32_t, Country>::const_iterator
			ipBlockItr(countryIPBlocks.begin());
		 ipBlockItr != countryIPBlocks.end();
		 ++ipBlockItr)
	{
		if (!pgBulkInserter.insert(NULL, ipBlockItr -> second.countryNumber,
								   ipBlockItr -> first,
								   ipBlockItr -> second.lastIP))
		{
			return false;
		}
		clock.incrementOperations();
	}
	if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
		return false;
	}
	clock.stop();
	return true;
}

int main(int, char *argv[]) {
	configuration conf;
	if (!conf.load("/usr/local/etc/infer.conf")) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}
	
	PostgreSQLConnection pgConn;
	string str;
	configuration::error status;
	status = conf.get(str, "host", "postgresql");
	if (status == configuration::OK) {
		pgConn.host(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.host" << endl;
		return 1;
	}
	status = conf.get(str, "hostaddr", "postgresql");
	if (status == configuration::OK) {
		pgConn.hostaddr(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.hostaddr" << endl;
		return 1;
	}
	status = conf.get(str, "port", "postgresql");
	if (status == configuration::OK) {
		pgConn.port(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.port" << endl;
		return 1;
	}
	status = conf.get(str, "dbname", "postgresql");
	if (status == configuration::OK) {
		pgConn.dbname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.dbname" << endl;
		return 1;
	}
	status = conf.get(str, "user", "postgresql");
	if (status == configuration::OK) {
		pgConn.user(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.user" << endl;
		return 1;
	}
	status = conf.get(str, "password", "postgresql");
	if (status == configuration::OK) {
		pgConn.password(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.password" << endl;
		return 1;
	}
	status = conf.get(str, "connect-timeout", "postgresql");
	if (status == configuration::OK) {
		pgConn.connect_timeout(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.connect-timeout" << endl;
		return 1;
	}
	status = conf.get(str, "options", "postgresql");
	if (status == configuration::OK) {
		pgConn.options(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.options" << endl;
		return 1;
	}
	status = conf.get(str, "sslmode", "postgresql");
	if (status == configuration::OK) {
		pgConn.sslmode(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.sslmode" << endl;
		return 1;
	}
	status = conf.get(str, "requiressl", "postgresql");
	if (status == configuration::OK) {
		pgConn.requiressl(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.requiressl" << endl;
		return 1;
	}
	status = conf.get(str, "krbsrvname", "postgresql");
	if (status == configuration::OK) {
		pgConn.krbsrvname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.krbsrvname" << endl;
		return 1;
	}
	status = conf.get(str, "gsslib", "postgresql");
	if (status == configuration::OK) {
		pgConn.gsslib(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.gsslib" << endl;
		return 1;
	}
	status = conf.get(str, "service", "postgresql");
	if (status == configuration::OK) {
		pgConn.service(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.service" << endl;
		return 1;
	}

	size_t flush_size;
	if (conf.get(flush_size, "flush-size", "country_map") != configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid flush-size" << endl;
		return 1;
	}

	vector<string> reserved_blocks;
	if (conf.get(reserved_blocks, "reserved-block", "country_map")
			!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid reserved-block" << endl;
		return 1;
	}
	
	vector<string> registries;
	if (conf.get(registries, "registry", "country_map") != configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid registry" << endl;
		return 1;
	}
	
	
	if (!pgConn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pgConn.error() << endl;
		return 1;
	}
	PGconn *postgreSQL(pgConn.connection());
	
	PGresult *result;
	ostringstream query;
	unordered_map <string, int16_t> countryCodeMap;
	pair <uint32_t, uint32_t> reservedRange;
	map <uint32_t, Country> countryIPBlocks;
	map <uint32_t, Country> newItr, adjacentItr;
	FILE *registryData;
	string line;
	char ch;
	vector <string> tokens;
	int16_t countryNumber = -1;
	uint32_t firstIP, lastIP;
	
	result = PQexecParams(postgreSQL,
						  "SELECT \"countryCode\", \"countryNumber\" "
							"FROM \"Maps\".\"countryNames\"",
						  0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}
	for (int row = 0; row < PQntuples(result); ++row) {
		countryCodeMap.insert(make_pair((char*)PQgetvalue(result, row, 0),
										ntohs(*(int16_t*)PQgetvalue(result,
																	row,
																	1))));
	}
	for (vector<string>::const_iterator reserved_block(reserved_blocks.begin());
		 reserved_block != reserved_blocks.end();
		 ++reserved_block)
	{
		cidrToRange(reservedRange, *reserved_block);
		updateCountryIPBlocks(countryIPBlocks, reservedRange.first,
							  reservedRange.second, countryNumber);
	}
	for (vector<string>::const_iterator registry(registries.begin());
		 registry != registries.end();
		 ++registry)
	{
		registryData = fetchGetURL(registry->c_str(), "p");
		while (getline(ch, line, registryData)) {
			if (line[0] == '#') {
				continue;
			}
			tokens.clear();
			explodeString(tokens, line, "|");
			if (tokens.size() != 7) {
				continue;
			}
			if (tokens[2] == "ipv4" && tokens[1] != "*") {
				countryNumber = countryCodeMap[tokens[1]];
				firstIP = textToIP(tokens[3]);
				lastIP = firstIP + strtoul(tokens[4].c_str(), NULL, 10) - 1;
				updateCountryIPBlocks(countryIPBlocks, firstIP, lastIP,
									  countryNumber);
			}
		}
		fclose(registryData);
		cout << *registry << " done." << endl;
	}
	if (!commitCountryIPBlocks(postgreSQL, flush_size, countryIPBlocks)) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}
	return 0;
}
