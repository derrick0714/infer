#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <openssl/bio.h>
#include <openssl/err.h>

#include "configuration.hpp"
#include "stringHelpers.h"
#include "hostPair.hpp"
#include "postgreSQL.h"
#include "clock.hpp"
#include "PostgreSQLConnection.hpp"

#include "routeServer.hpp"
#include "as.hpp"

#define AS_MAP_TABLE_SCHEMA "\"asn\" uint16 NOT NULL, \
						\"firstIP\" uint32 NOT NULL, \
						\"lastIP\" uint32 NOT NULL, \
						PRIMARY KEY (\"firstIP\", \"lastIP\")"

using namespace std;

vector <RouteServer> routeServers;
map <AS, uint16_t> asMap;
pthread_mutex_t displayLock, asMapLock;

bool getline(BIO *socket, char *buffer, size_t bufferSize, string &stringBuffer,
						 string &line, string delimiter = "\n") {
	int bytesRead;
	line.clear();
	size_t delimiterPosition = stringBuffer.find(delimiter);
	if (delimiterPosition != string::npos) {
		line = stringBuffer.substr(0, delimiterPosition);
		stringBuffer.erase(0, delimiterPosition + delimiter.length());
		return true;
	}
	while ((bytesRead = BIO_read(socket, buffer, bufferSize)) > 0) {
		stringBuffer.append(buffer, bytesRead);
		delimiterPosition = stringBuffer.find(delimiter);
		if (delimiterPosition != string::npos) {
			if (stringBuffer.find("--More--") != string::npos &&
					BIO_write(socket, " ", 1) < 1) {
				return false;
			}
			line = stringBuffer.substr(0, delimiterPosition);
			stringBuffer.erase(0, delimiterPosition + delimiter.length());
			return true;
		}
	}
	if (stringBuffer.length()) {
		line = stringBuffer;
		stringBuffer.clear();
		return true;
	}
	return false;
}

void updateASMap(map <AS, uint16_t> &asMap, const uint32_t &firstIP,
								 const uint32_t &lastIP, const uint16_t &asn) {
	AS as(firstIP, lastIP);
	map <AS, uint16_t>::iterator newItr = asMap.insert(make_pair(as, asn)).first,
															 adjacentItr;
	bool updateASMap = false;
	--(adjacentItr = newItr);
	if (asn == adjacentItr -> second &&
			firstIP <= adjacentItr -> first.lastIP + 1 && adjacentItr != newItr) {
		updateASMap = true;
		if (firstIP > adjacentItr -> first.firstIP) {
			as.firstIP = adjacentItr -> first.firstIP;
		}
		if (lastIP < adjacentItr -> first.lastIP) {
			as.lastIP = adjacentItr -> first.lastIP;
		}
		asMap.erase(adjacentItr);
	}
	++(adjacentItr = newItr);
	if (asn == adjacentItr -> second &&
			adjacentItr -> first.firstIP <= as.lastIP + 1 && adjacentItr != newItr) {
		updateASMap = true;
		if (as.firstIP > adjacentItr -> first.firstIP) {
			as.firstIP = adjacentItr -> first.firstIP;
		}
		if (as.lastIP < adjacentItr -> first.lastIP) {
			as.lastIP = adjacentItr -> first.lastIP;
		}
		asMap.erase(adjacentItr);
	}
	if (updateASMap) {
		asMap.erase(newItr);
		asMap.insert(make_pair(as, asn));
	}
}

void *getRouteServerData(void *_routeServer) {
	RouteServer *routeServer = (RouteServer*)_routeServer;
	char buffer[4096];
	string stringBuffer, line, cidrBlock;
	vector <string> octets;
	size_t index, slashPosition, bits, octetIndex, asnStart, asnEnd;
	uint32_t firstIP, lastIP;
	uint16_t asn;
	routeServer -> socket = BIO_new_connect((char*)(routeServer -> address + ":23").c_str());
	if (BIO_do_connect(routeServer -> socket) < 1) {
		ERR_print_errors_fp(stderr);
		BIO_free(routeServer -> socket);
		routeServer -> lock();
		routeServer -> status = false;
		routeServer -> unlock();
		return NULL;
	}
	if (BIO_write(routeServer -> socket,
								"\xff\xfd\x01\xff\xfa\x1f\x00\xa7\x00\x36\xff\xf0", 12) < 1 ||
			BIO_write(routeServer -> socket, "show ip bgp\r\n", 13) < 1) {
		ERR_print_errors_fp(stderr);
		BIO_free(routeServer -> socket);
		routeServer -> lock();
		routeServer -> status = false;
		routeServer -> unlock();
		return NULL;
	}
	while (getline(routeServer -> socket, buffer, sizeof(buffer), stringBuffer,
								 line)) {
		routeServer -> lock();
		routeServer -> lastReadTime = time(NULL);
		index = 0;
		/* We find the first digit on the line. */
		while (!isdigit(line[index]) && index < line.length()) {
			++index;
		}
		if (index != line.length()) {
			cidrBlock.clear();
			octets.clear();
			/*
			 * We copy digits, dots, and slashes to cidrBlock, starting from the first
			 * digit in the line, until we encounter another type of character.
			 */
			while (isdigit(line[index]) || line[index] == '.' || line[index] == '/') {
				cidrBlock += line[index];
				++index;
			}
			slashPosition = cidrBlock.rfind('/');
			explodeString(octets, cidrBlock.substr(0, slashPosition), ".");
			if (octets.size() == 4) {
				/*
				 * If we are given the number of bits in the CIDR allocation, we use it.
				 */
				if (slashPosition != string::npos) {
					bits = strtoul(cidrBlock.substr(slashPosition + 1).c_str(), NULL, 10);
				}
				/*
				 * Otherwise, we start with 32 bits and decrement by eight for each
				 * consecutive zero-value octet encountered while iterating through the
				 * octets in reverse.
				 */
				else {
					bits = 32;
					octetIndex = 3;
					while (octets[octetIndex] == "0") {
						bits -= 8;
						--octetIndex;
					}
				}
				if (bits != 32) {
					firstIP = pton(cidrBlock.substr(0, slashPosition));
					lastIP = firstIP + (uint32_t)pow((long double)2, (long double)(32 - bits)) - 1;
					asnEnd = line.length();
					/* We find the last digit on the line. */
					while (!isdigit(line[asnEnd]) && asnEnd > index) {
						--asnEnd;
					}
					if (asnEnd > index) {
						asnStart = asnEnd;
						/*
						 * We find the first non-digit character before the last digit on
						 * the line. The ASN is the number in between.
						 */
						while (isdigit(line[asnStart])) {
							--asnStart;
						}
						asn = strtoul(line.substr(asnStart + 1, asnEnd - asnStart).c_str(),
													NULL, 10);
						if (asn) {
							pthread_mutex_lock(&asMapLock);
							updateASMap(asMap, firstIP, lastIP, asn);
							pthread_mutex_unlock(&asMapLock);
						}
					}
				}
			}
		}
		routeServer -> unlock();
	}
	ERR_print_errors_fp(stderr);
	routeServer -> lock();
	BIO_free(routeServer -> socket);
	routeServer -> status = false;
	routeServer -> unlock();
	return NULL;
}

void *checkForTimeouts(void *_timeout) {
	time_t timeout = *(time_t*)_timeout, currentTime;
	size_t numActiveThreads;
	do {
		sleep(1);
		numActiveThreads = 0;
		currentTime = time(NULL);
		for (size_t index = 0; index < routeServers.size(); ++index) {
			routeServers[index].lock();
			if (routeServers[index].status) {
				++numActiveThreads;
				if (routeServers[index].status && routeServers[index].lastReadTime &&
						routeServers[index].lastReadTime < currentTime - timeout) {
					cout << routeServers[index].address << " timed out." << endl;
					routeServers[index].status = false;
					pthread_cancel(routeServers[index].thread);
				}
			}
			routeServers[index].unlock();
		}
	} while (numActiveThreads);
	return NULL;
}

bool commitASMap(PGconn *postgreSQL, size_t flushSize,
								 const map <AS, uint16_t> &asMap) {
	Clock clock("Inserted", "rows");
	PGBulkInserter pgBulkInserter(postgreSQL, "Maps", "asIPBlocks", flushSize,
																"%ud, %ud, %ud");
	if (!preparePGTable(postgreSQL, "Maps", "asIPBlocks", AS_MAP_TABLE_SCHEMA)) {
		return false;
	}
	clock.start();
	for (map <AS, uint16_t>::const_iterator asMapItr = asMap.begin();
			 asMapItr != asMap.end(); ++asMapItr) {
		if (!pgBulkInserter.insert(NULL, asMapItr -> second,
															 asMapItr -> first.firstIP,
															 asMapItr -> first.lastIP)) {
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
	
	time_t timeout;
	if (conf.get(timeout, "timeout", "as_map") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid timeout" << endl;
		return 1;
	}

	size_t flush_size;
	if (conf.get(flush_size, "flush-size", "as_map") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid flush-size" << endl;
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

	vector<string> server_names;
	if (conf.get(server_names, "route-server", "as_map") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid route-server" << endl;
		return 1;
	}
	
	if (!pgConn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pgConn.error() << endl;
		return 1;
	}
	PGconn *postgreSQL(pgConn.connection());
	
	pthread_t timeoutThread;
	pthread_mutex_init(&displayLock, NULL);
	pthread_mutex_init(&asMapLock, NULL);
	ERR_load_BIO_strings();

	pthread_mutex_init(&displayLock, NULL);

	routeServers.resize(server_names.size());
	for (size_t routeServer = 0; routeServer < routeServers.size(); ++routeServer) {
		routeServers[routeServer].address = server_names[routeServer];
		pthread_create(&(routeServers[routeServer].thread), NULL,
									 &getRouteServerData, (void*)&(routeServers[routeServer]));
	}
	pthread_create(&timeoutThread, NULL, &checkForTimeouts, (void*)&timeout);
	pthread_join(timeoutThread, NULL);

	if (!commitASMap(postgreSQL, flush_size, asMap)) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}
	return 0;
}
