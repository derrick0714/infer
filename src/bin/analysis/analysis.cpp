#include <iostream>
#include <sstream>
#include <vector>
#include <tr1/unordered_set>
#include <unistd.h>

#include "configuration.hpp"
#include "address.h"
#include "postgreSQL.h"
#include "modules.hpp"
#include "filesystem.hpp"
#include "httpFlow.hpp"
#include "hostPair.hpp"
#include "connectionInitiator.hpp"
#include "ipInformation.h"
#include "clock.hpp"
#include "bsdProcessStats.h"
#include "reputationInformation.hpp"
#include "seenIPsInformation.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "FlatFileReader.hpp"
#include "LiveIP.hpp"
#include "EnumeratedFileReader.hpp"
#include "PostgreSQLConnection.hpp"
#include "print_error.hpp"

#define	PROCESS_NAME "infer_analysis"
#define	LIVE_IPS_TABLE_STRUCTURE "\"ip\" uint32 NOT NULL, \
									\"mac\" TEXT NOT NULL, \
									PRIMARY KEY (\"ip\")"

#define	NEOFLOW_STAGE_TYPE		0
#define	HTTP_STAGE_TYPE			1

using namespace std;
using namespace tr1;

void updateLiveIPs(unordered_map <uint32_t, string> *liveIPs,
									 const char *data) {
	for (uint32_t liveIP = 0; liveIP < *(uint32_t*)(data + 8); ++liveIP) {
		liveIPs -> insert(make_pair(*(uint32_t*)(data + 12 + (liveIP * 10)),
									string(data + 16 + (liveIP * 10), 6)));
	}
}

bool commitLiveIPs(PGconn *postgreSQL, const char *tableName, size_t &flushSize,
									 unordered_map <uint32_t, string> *liveIPs) {
	Clock clock("Inserted", "rows");
	if (!preparePGTable(postgreSQL, LIVE_IPS_SCHEMA_NAME, tableName,
						LIVE_IPS_TABLE_STRUCTURE)) {
		return false;
	}
	PGBulkInserter pgBulkInserter(postgreSQL, LIVE_IPS_SCHEMA_NAME, tableName,
								flushSize, "%ud, %s");
	cout << "Updating PostgreSQL database with live IPs" << endl;
	clock.start();
	for (unordered_map <uint32_t, string>::iterator liveIPItr(liveIPs -> begin());
			 liveIPItr != liveIPs -> end(); ++liveIPItr)
	{
		if (!pgBulkInserter.insert(NULL, liveIPItr -> first,
								   ethernetToText(liveIPItr -> second.c_str()).c_str()))
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

void getLocalNetworks(vector <pair <uint32_t, uint32_t> > &localNetworks,
											const string &_localNetworks)
{
	vector <string> __localNetworks = explodeString(_localNetworks, " ");
	for (size_t localNetwork = 0;
		 localNetwork < __localNetworks.size();
		 ++localNetwork)
	{
		localNetworks.push_back(cidrToRange(__localNetworks[localNetwork]));
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cerr << "usage: " << argv[0] << " date" << endl;
		return 1;
	}

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
	if (conf.get(flush_size, "flush-size", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid flush-size" << endl;
		return 1;
	}

	string stage_types;
	if (conf.get(stage_types, "stages", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid stages" << endl;
		return 1;
	}

	vector<string> stages;
	if (conf.get(stages, "stage", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid stage" << endl;
		return 1;
	}

	string local_networks;
	if (conf.get(local_networks, "local-networks", "analysis", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid local-networks" << endl;
		return 1;
	}

	size_t udp_timeout;
	if (conf.get(udp_timeout, "udp-timeout", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid udp_timeout" << endl;
		return 1;
	}

	size_t start_hour;
	if (conf.get(start_hour, "start-hour", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid start_hour" << endl;
		return 1;
	}

	size_t end_hour;
	if (conf.get(end_hour, "end-hour", "analysis") != configuration::OK) {
		cerr << argv[0] << ": missing or invalid end_hour" << endl;
		return 1;
	}

	string data_directory;
	if (conf.get(data_directory, "data-directory", "analysis", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}
	
	string module_path;
	if (conf.get(module_path, "module-path", "analysis")
			!= configuration::OK)
	{
		cerr << argv[0] << ": module-path required" << endl;
		return 1;
	}
	
	string module_prefix;
	if (conf.get(module_prefix, "module-prefix", "analysis")
			!= configuration::OK)
	{
		cerr << argv[0] << ": module-prefix required" << endl;
		return 1;
	}
	
	string query_socket_path;
	if (conf.get(query_socket_path, "socket-path", "query_manager") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid query_manager socket-path"
			 << endl;
		return 1;
	}

	size_t query_socket_timeout;
	if (conf.get(query_socket_timeout, "socket-timeout", "query_manager") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid query_manager socket-timeout"
			 << endl;
		return 1;
	}

	SharedState sharedState;
	map <string, size_t> stageTypeMap;
	vector <string> stageTypes;
	vector <vector <Module> > modules;
	int insertedRows;
	size_t file(0);
	size_t totalFiles(0);
	size_t hour;
	size_t stageType;
	size_t totalInsertedRows(0);
	Clock clock("Read" , "records");
	IPInformation ipInformation;

	if (!pgConn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pgConn.error() << endl;
		return 1;
	}
	PGconn *postgreSQL(pgConn.connection());

	if (PQresultStatus(PQexec(postgreSQL, "SET CLIENT_MIN_MESSAGES TO WARNING")) != PGRES_COMMAND_OK) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}

	sharedState.postgreSQL = postgreSQL;
	sharedState.flushSize = flush_size;
	sharedState.date = argv[1];
	sharedState.localNetworks = new vector <std::pair <uint32_t, uint32_t> >;
	getLocalNetworks(*(sharedState.localNetworks), local_networks);
	sharedState.liveIPs = new unordered_map <uint32_t, string>;
	sharedState.connectionInitiators = new ConnectionInitiators;
	sharedState.connectionInitiators -> setUDPTimeout(udp_timeout);
	sharedState.connectionInitiators -> setLocalNetworks(sharedState.localNetworks);
	sharedState.ipInformation = new IPInformation;
	if (!sharedState.ipInformation -> initialize(postgreSQL)) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}

	if (start_hour > 23 || end_hour > 24) {
		cerr << argv[0] << ": hour range misconfiguration! Exiting..." << endl;
		return 1;
	}

	struct tm _tm;
	if (strptime(argv[1], "%Y-%m-%d", &_tm) == NULL) {
			cerr << argv[0] << ": invalid date: " << argv[1] << endl;
		return 1;
	}

	struct tm tm_begin, tm_end;
	memset(&tm_begin, 0, sizeof(tm_begin));
	memset(&tm_end, 0, sizeof(tm_end));

	tm_begin.tm_isdst = tm_end.tm_isdst = -1;
	tm_begin.tm_year = tm_end.tm_year = _tm.tm_year;
	tm_begin.tm_mon = tm_end.tm_mon = _tm.tm_mon;
	tm_begin.tm_mday = tm_end.tm_mday = _tm.tm_mday;
	tm_begin.tm_hour = start_hour;
	tm_end.tm_hour = end_hour;

	time_t time_begin, time_end;
	time_begin = mktime(&tm_begin);
	time_end = mktime(&tm_end);

	TimeStamp analysis_begin, analysis_end;
	analysis_begin.set(time_begin, 0);
	analysis_end.set(time_end, 0);

	cerr << "about to read live_ips data" << endl;
	/* Reads live IPs from live IP data. */
	StrftimeReadEnumerator readEnumerator;
	boost::shared_ptr<StrftimeReadEnumerator> liveIPsEnumerator(new StrftimeReadEnumerator);
	liveIPsEnumerator->init(data_directory,
												string("%Y/%m/%d/live_ips_%H"),
												analysis_begin,
												analysis_end);

	if (liveIPsEnumerator->begin() == liveIPsEnumerator->end()) {
		cerr << argv[0] << ": no live_ips data for " << argv[1] << endl;
		return 1;
	}
	cerr << "preparePGTable..." << endl;
	if (!preparePGTable(postgreSQL, DATA_SIZE_SCHEMA_NAME, argv[1],
											DATA_SIZE_TABLE_SCHEMA, "dataType", "live_ips")) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	cerr << "insertPGRow..." << endl;
	if (!insertPGRow(postgreSQL, DATA_SIZE_SCHEMA_NAME, argv[1], "%s, %ul",
									 "live_ips", getDataSize(liveIPsEnumerator->begin(), liveIPsEnumerator->end()))) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	cerr << "loading modules..." << endl;
	if (!loadModules(modules,
					 stages,
					 module_path,
					 module_prefix,
					 conf))
	{
		print_error(argv[0], 
				   modules[modules.size() - 1]
				   		[modules[modules.size() - 1].size() - 1].error());
		return 1;
	}
	cerr << "updating live IPs" << endl;
	LiveIP liveIP;
	ErrorStatus errorStatus;
	EnumeratedFileReader<FlatFileReader<LiveIP>, StrftimeReadEnumerator> liveIPReader;
	if (liveIPReader.init(liveIPsEnumerator) != E_SUCCESS) {
			cerr << "Error initializing liveIPReader!" << endl;
		return 1;
	}
	while ((errorStatus = liveIPReader.read(liveIP)) == E_SUCCESS) {
			sharedState.liveIPs->insert(make_pair(liveIP.ip(), string(reinterpret_cast<char *>(& liveIP.mac()), 6)));
	}
	if (errorStatus != E_EOF) {
			cerr << "error reading liveIPs!" << endl;
		return 1;
	}

	/* Stores live IPs in PostgreSQL. */
	cerr << "storing live IPs in postgreSQL" << endl;
	if (!commitLiveIPs(postgreSQL, argv[1], sharedState.flushSize,
										 sharedState.liveIPs)) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}
	sharedState.roles = new std::tr1::unordered_multimap <uint32_t, Role>;
	if (!preparePGTable(postgreSQL, PROCESS_STATS_SCHEMA_NAME, argv[1],
											PROCESS_STATS_TABLE_SCHEMA, "processName",
											PROCESS_NAME)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME, 0, 0)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	
	sharedState.nameResolution = new NameResolution(query_socket_path,
													query_socket_timeout);
	cout<<"#derrick log# query_socket_path:"<<query_socket_path<<endl;
	cout<<"#derrick log# query_socket_path:"<<query_socket_timeout<<endl;
	if (sharedState.nameResolution -> establishConnection()) {
		cout << "Successfully initialized NameResolution and connected to socket." << endl;
	}
	else {
		cout << "NameResolution: Error connecting to Query Manager." << endl;
		return 1;
	}

	sharedState.nameResolution->load(analysis_begin, analysis_end);
	sharedState.nameResolution->waitForData();

	sharedState.seenIPsInformation = new SeenIPsInformation();
	cout << "SeenIPsInformation initialized..." << endl;
	sharedState.reputationInformation = new ReputationInformation(sharedState.postgreSQL,
																sharedState.ipInformation,
																sharedState.nameResolution,
																sharedState.seenIPsInformation,
																sharedState.date);
	cout << "ReputationInformation initialized..." << endl;
	stageTypeMap.insert(make_pair("neoflow", NEOFLOW_STAGE_TYPE));
	stageTypeMap.insert(make_pair("http", HTTP_STAGE_TYPE));
	stageTypes = explodeString(stage_types, " ");
	for (size_t stage = 0; stage < modules.size(); ++stage) {
		stageType = stageTypeMap[stageTypes[stage]];
		readEnumerator.init(data_directory,
												string("%Y/%m/%d/").append(stageTypes[stage]).append("_%H"),
												analysis_begin,
												analysis_end);

		for (StrftimeReadEnumerator::iterator curFile(readEnumerator.begin());
				 curFile != readEnumerator.end();
				 ++curFile)
		{
				cerr << "Files: " << curFile->string() << endl;
		}
		cerr << endl;
		if (readEnumerator.begin() == readEnumerator.end()) {
			cerr << argv[0] << ": no " << stageTypes[stage] << " data for " << argv[1] << endl;
			return 1;
		}
		for (StrftimeReadEnumerator::const_iterator it(readEnumerator.begin());
				 it != readEnumerator.end();
				 ++it)
		{
				++totalFiles;
		}
		if (!preparePGTable(postgreSQL, DATA_SIZE_SCHEMA_NAME, argv[1],
												DATA_SIZE_TABLE_SCHEMA, "dataType",
												stageTypes[stage])) {
			cerr << PQerrorMessage(postgreSQL);
			return 1;
		}
		if (!insertPGRow(postgreSQL, DATA_SIZE_SCHEMA_NAME, argv[1], "%s, %ul",
										 stageTypes[stage].c_str(),
										 getDataSize(readEnumerator.begin(), readEnumerator.end()))) {
			cerr << PQerrorMessage(postgreSQL);
			return 1;
		}
	}
	for (size_t stage = 0; stage < modules.size(); ++stage) {
			cout << "Stage " << stage << ':' << endl;
		readEnumerator.init(data_directory,
							string("%Y/%m/%d/").append(stageTypes[stage]).append("_%H"),
							analysis_begin,
							analysis_end);

		for (size_t module = 0; module < modules[stage].size(); ++module) {
			cout << "Initializing module '" << modules[stage][module].name() << "'" << endl;
			if (!modules[stage][module].initialize(sharedState,
									 			   modules[stage][module].moduleState))
			{
				print_error(argv[0], modules[stage][module].name(), "initialize() failed");
				return 1;
			}
		}
		cout << "All modules initialized..." << endl;
		hour = 0;
		clock.start();
		for (StrftimeReadEnumerator::iterator curFile(readEnumerator.begin());
				 curFile != readEnumerator.end();
				 ++curFile)
		{
			cout << "Reading from " << curFile->string() << endl;
			if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME,
					((double)(file) / totalFiles) * 100, totalInsertedRows))
			{
				cerr << PQerrorMessage(postgreSQL);
				return 1;
			}
			++file;
			hour = strtoul(curFile->string().substr(curFile->string().length() - 2).c_str(), NULL, 10);

			switch (stageType) {
				case NEOFLOW_STAGE_TYPE:
					{
						FlatFileReader<FlowStats> reader;
						FlowStats flowStats;
						if (reader.open(*curFile) != E_SUCCESS) {
							cerr << "error opening \"" << curFile->string() << '"' << endl;
							return 1;
						}
						while ((errorStatus = reader.read(flowStats)) == E_SUCCESS) {
							for (size_t module = 0; module < modules[stage].size(); ++module) {
								modules[stage][module].aggregate(&flowStats, hour);
								clock.incrementOperations();
							}
						}
						if (errorStatus != E_EOF) {
							cerr << "error reading from \"" << curFile->string() << '"' << endl;
							return 1;
						}
						if (reader.close() != E_SUCCESS) {
							cerr << "error closing \"" << curFile->string() << '"' << endl;
							return 1;
						}
					}
					break;
				default:
					cerr << "unknown stage type." << endl;
					return 1;
					break;
			};
			clock.stop();
		}
		if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME,
				((double)(file) / totalFiles) * 100, totalInsertedRows))
		{
			cerr << PQerrorMessage(postgreSQL);
			return 1;
		}
		for (size_t module = 0; module < modules[stage].size(); ++module) {
			insertedRows = modules[stage][module].commit(pgConn,
											 sharedState.flushSize,
											 argv[1]);
			if (insertedRows == -1) {
				cerr << modules[stage][module].name() << ": commit() failed!" << endl;
				cerr << argv[0] << ": " << PQerrorMessage(postgreSQL) << endl;
				return 1;
			}
			totalInsertedRows += insertedRows;
		}
	}
	if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME, 100,
													totalInsertedRows)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	return 0;
}
