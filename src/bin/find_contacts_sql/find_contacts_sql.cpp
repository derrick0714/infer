#include <iostream>
#include <string>
#include <vector>

#include "configuration.hpp"
#include "filesystem.hpp"
#include "hostPair.hpp"
#include "postgreSQL.h"
#include "StrftimeReadEnumerator.hpp"
#include "FlatFileReader.hpp"
#include "PostgreSQLConnection.hpp"

using namespace std;

struct ContactQuery {
	string queryID;
	uint32_t ip;
	uint32_t startTime;
	uint32_t endTime;
	vector <string> files;
};

ContactQuery query;
PGconn *postgreSQL;

void usage(string programName) {
	cerr << "usage: " << programName << " query_id"
										" ip_address"
										" start_date"
										" start_time"
										" end_date"
										" end_time" << endl;
}

bool processNeoflow(const FlowStats *flowStats) {
	if ((flowStats -> sourceIP() != query.ip && 
			flowStats -> destinationIP() != query.ip) ||
		flowStats -> startTime().seconds() >= query.endTime ||
		flowStats -> endTime().seconds() < query.startTime)
	{
		return true;
	}

	// update number of results in sql
	static ostringstream pgQuery;
	static PGresult *result;

	pgQuery.str("");
	pgQuery << "UPDATE \"Indexes\".\"contactsQueries\" SET \"numResults\" = "
			<< "\"numResults\" + '1' WHERE \"id\" = '" << query.queryID << '\'';
	result = PQexec(postgreSQL, pgQuery.str().c_str());
	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
		cerr << PQerrorMessage(postgreSQL);
		PQclear(result);
		return false;
	}
	PQclear(result);

	// actually insert the result into postgres
	if (!insertPGRow(postgreSQL, "ContactsQueries", query.queryID, 
					 "%ud, %ud, %ud, %ud, %ud, %ud, %ud",
					 (unsigned int) flowStats -> protocol(),
					 flowStats -> sourceIP(),
					 flowStats -> destinationIP(),
					 flowStats -> sourcePort(),
					 flowStats -> destinationPort(),
					 flowStats -> startTime().seconds(),
					 flowStats -> endTime().seconds()))
	{
		cerr << PQerrorMessage(postgreSQL);
		return false;
	}

	/*
	cout << (int) flowStats -> protocol << ' '
		 << ntop(flowStats -> sourceIP) << ' '
		 << ntop(flowStats -> destinationIP) << ' '
		 << flowStats -> sourcePort << ' '
		 << flowStats -> destinationPort << ' '
		 << getDisplayTime(flowStats -> startTime) << ' '
		 << getDisplayTime(flowStats -> endTime) << endl;
	*/

	return true;
}

int main(int argc, char **argv) {
	if (argc != 7) {
		usage(argv[0]);
		return 1;
	}

	query.queryID.assign(argv[1]);
	query.ip = pton(argv[2]);
	query.startTime = getUNIXTime(explodeDate(argv[3], argv[4]));
	query.endTime = getUNIXTime(explodeDate(argv[5], argv[6]));
/*
		query.startTime += getUTCOffset();
		query.endTime += getUTCOffset();
*/

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

	string data_directory;
	if (conf.get(data_directory, "data-directory", "find_contacts_sql", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	if (!pgConn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pgConn.error() << endl;
		return 1;
	}
	postgreSQL = pgConn.connection();
	
	time_t foo;
	foo = query.startTime;
	cout << ctime(&foo);
	foo = query.endTime;
	cout << ctime(&foo);
	if (PQstatus(postgreSQL) != CONNECTION_OK) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		return 1;
	}
	ostringstream pgQuery;
	PGresult * result;

	pgQuery << "UPDATE \"Indexes\".\"contactsQueries\" SET \"pid\" = '" << getpid()
			<< "' WHERE \"id\" = '" << query.queryID << '\'';
	result = PQexec(postgreSQL, pgQuery.str().c_str());
	pgQuery.str("");
	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		PQclear(result);
		return 1;
	}

	StrftimeReadEnumerator readEnumerator(data_directory,
										  "%Y/%m/%d/neoflow_%H",
										  TimeStamp(query.startTime, 0),
										  TimeStamp(query.endTime, 0));
	if (readEnumerator.begin() == readEnumerator.end()) {
		cerr << argv[0] << ": error: No neoflow data in range" << endl;
		for (size_t i = 0; i < query.files.size(); ++i) {
				cout << query.files[i] << endl;
		}
		pgQuery << "UPDATE \"Indexes\".\"contactsQueries\" SET \"duration\" = '"
			  << 0 << "', \"timeLeft\" = NULL, "
			  << "\"percentComplete\" = '100', \"status\" = '1' "
			  << "WHERE \"id\" = '" << query.queryID << '\'';
		result = PQexec(postgreSQL, pgQuery.str().c_str());
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
			PQclear(result);
		}
		return 1;
	}

	FlatFileReader<FlowStats> reader;
	FlowStats flowStats;
	ErrorStatus errorStatus;
	size_t curFile = 0;
	for (StrftimeReadEnumerator::iterator it(readEnumerator.begin());
		 it != readEnumerator.end();
		 ++it)
	{
		if (reader.open(*it) != E_SUCCESS) {
			cerr << argv[0] << ": error: Error opening '" << *it << "'" << endl;
			return 1;
		}
		while ((errorStatus = reader.read(flowStats)) == E_SUCCESS) {
			if (!processNeoflow(&flowStats)) {
				cerr << argv[0] << ": error: Error processing neoflow record" << endl;
				return 1;
			}
		}
		if (errorStatus != E_EOF) {
			cerr << argv[0] << ": error: Error reading from '" << *it << "'" << endl;
			return 1;
		}
		
		++curFile;
		// update the percent complete in postgres
		pgQuery.str("");
		pgQuery << "UPDATE \"Indexes\".\"contactsQueries\" SET \"percentComplete\" = '"
				<< ((double)(curFile - 1) / query.files.size()) * 100
				<< "' WHERE \"id\" = '" << query.queryID << '\'';
		result = PQexec(postgreSQL, pgQuery.str().c_str());
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << PQerrorMessage(postgreSQL);
			PQclear(result);
		}

		if (reader.close() != E_SUCCESS) {
			cerr << argv[0] << ": error: Error closing '" << *it << "'" << endl;
			return 1;
		}
	}

	pgQuery.str("");
	pgQuery << "SELECT \"startTime\", \"resumeTime\", \"duration\" FROM "
		  << "\"Indexes\".\"contactsQueries\" WHERE \"id\" = '" << query.queryID
		  << '\'';
	result = PQexecParams(postgreSQL, pgQuery.str().c_str(), 0,
						  NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
		PQclear(result);
	} else if (PQntuples(result) == 1) {
		uint32_t _time, startTime, resumeTime, duration;
		_time = time(NULL);
		if (PQgetisnull(result, 0, 1) == 1) {
			startTime = ntohl(*(uint32_t*)PQgetvalue(result, 0, 0));
			duration = _time - startTime;
		} else {
			resumeTime = ntohl(*(uint32_t*)PQgetvalue(result, 0, 1));
			duration = ntohl(*(uint32_t*)PQgetvalue(result, 0, 2));
			duration = duration + _time - resumeTime;
		}
		/*
		* Updates the Contacts query index with the program's final duration, time
		* left (NULL, as it no longer applies), percent completion (100), and
		* status (completed).
		*/
		pgQuery.str("");
		pgQuery << "UPDATE \"Indexes\".\"contactsQueries\" SET \"duration\" = '"
			  << duration << "', \"timeLeft\" = NULL, "
			  << "\"percentComplete\" = '100', \"status\" = '1' "
			  << "WHERE \"id\" = '" << query.queryID << '\'';
		result = PQexec(postgreSQL, pgQuery.str().c_str());
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << argv[0] << ": " << PQerrorMessage(postgreSQL);
			PQclear(result);
		}
	}

	return 0;
}
