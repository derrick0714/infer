#include <iostream>

#include "shared/PostgreSQLConnection.hpp"
#include "shared/PostgreSQLWriter.hpp"
#include "shared/HBFHTTPResult.hpp"
#include "shared/PayloadSearchManager.hpp"
#include "shared/PayloadSearchManagerArguments.h"
#include "shared/PayloadSearchManagerConfiguration.h"
//#include "shared/OstreamWriter.hpp"

using namespace std;
using namespace vn::arl::shared;

int main(int argc, char **argv) {
	PayloadSearchManagerArguments args(argc, argv);

	if (!args) {
		cerr << "Error: " << args.error() << endl << endl
			 << args << endl;

		return 1;
	}

	PayloadSearchManagerConfiguration conf(args.configFile());
	if (!conf) {
		cerr << "Error: PayloadSearchManagerConfiguration: " << conf.error() 
			 << endl;
		return 1;
	}

	PostgreSQLConnection pgConn;
	pgConn.host(conf.pgHost());
	pgConn.hostaddr(conf.pgHostaddr());
	pgConn.port(conf.pgPort());
	pgConn.dbname(conf.pgDbname());
	pgConn.user(conf.pgUser());
	pgConn.password(conf.pgPassword());
	pgConn.connect_timeout(conf.pgConnectTimeout());
	pgConn.options(conf.pgOptions());
	pgConn.sslmode(conf.pgSslmode());
	pgConn.requiressl(conf.pgRequiressl());
	pgConn.krbsrvname(conf.pgKrbsrvname());
	pgConn.gsslib(conf.pgGsslib());
	pgConn.service(conf.pgService());

	PostgreSQLWriter <HBFHTTPResult> hbfhttpWriter(pgConn,
												   conf.pgSchema(),
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
					conf.maxMTU(),
					conf.maxFlows(),
					conf.threadCount());

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
