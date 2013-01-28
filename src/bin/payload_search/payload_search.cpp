#include <iostream>

#include "PostgreSQLConnection.hpp"
#include "PostgreSQLWriter.hpp"
#include "HBFHTTPResult.hpp"
#include "PayloadSearchManager.hpp"
#include "PayloadSearchManagerArguments.h"
#include "configuration.hpp"

using namespace std;

int main(int argc, char **argv) {
	PayloadSearchManagerArguments args(argc, argv);

	if (!args) {
		cerr << "Error: " << args.error() << endl << endl
			 << args << endl;

		return 1;
	}


	configuration conf;
	if (!conf.load(args.configFile().string())) {
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

	string result_schema;
	if (conf.get(result_schema, "result-schema", "payload_search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": result-schema required" << endl;
		return 1;
	}

	size_t max_mtu;
	if (conf.get(max_mtu, "max-mtu", "payload_search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": max-mtu required" << endl;
		return 1;
	}

	size_t max_flows;
	if (conf.get(max_flows, "max-flows", "payload_search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": max-flows required" << endl;
		return 1;
	}

	size_t thread_count;
	if (conf.get(thread_count, "thread-count", "payload_search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": thread-count required" << endl;
		return 1;
	}

	PostgreSQLWriter <HBFHTTPResult> hbfhttpWriter(pgConn,
												   result_schema,
												   args.queryID());
	if (!hbfhttpWriter) {
		cerr << "Error: PostgreSQLWriter: " << hbfhttpWriter.error() << endl;
		return 1;
	}
//	OstreamWriter <HBFHTTPResult> hbfhttpWriter(std::cout);

	PayloadSearchManager
		<PostgreSQLWriter
//		<OstreamWriter
			<HBFHTTPResult>
		> searchMan(&hbfhttpWriter,
					args.inputDir(),
					args.startTime(),
					args.endTime(),
					args.inputData(),
					args.queryLength(),
					args.matchLength(),
					args.ipv4FlowMatcher(),
					max_mtu,
					max_flows,
					thread_count);

	if (!searchMan) {
		cerr << searchMan.error() << endl;
		return 1;
	}

	if (searchMan.run() != 0) {
		cerr << searchMan.error() << endl;
		return 1;
	}
	
	return 0;
}
