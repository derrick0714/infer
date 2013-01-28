#include <iostream>
#include <string>
#include <tr1/unordered_set>

#include <boost/program_options.hpp>

#include "configuration.hpp"
#include "stringHelpers.h"
#include "PostgreSQLConnection.hpp"

using namespace std;
using namespace tr1;
using namespace boost;
using namespace boost::program_options;

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("date", value<string>(), "the date (YYYY-mm-dd)")
	;

	positional_options_description p;
	p.add("date", 1);
	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).positional(p).run(), vm);
	}
	catch (const boost::program_options::error &e) {
		cerr << e.what() << endl;
		return 1;
	}
	notify(vm);

	if (vm.count("help")) {
		cout << desc_gen << endl;
		return 0;
	}

	bool decimal(false);
	if (vm.count("decimal")) {
		decimal = true;
	}

	if (!vm.count("date")) {
		cerr << "Error: date is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	string date(vm["date"].as<string>());


	struct tm _tm;
	if (strptime(vm["date"].as<string>().c_str(), "%Y-%m-%d", &_tm) == NULL) {
		cerr << "Error: invalid date: " << vm["date"].as<string>() << endl;
		return 1;
	}

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string external_contacts_schema;
	if (conf.get(external_contacts_schema, "external-contacts-schema", "comm_channel_fanin")
			!= configuration::OK)
	{
		cerr << argv[0] << ": external-contacts-schema required" << endl;
		return 1;
	}

	string comm_channels_schema;
	if (conf.get(comm_channels_schema, "comm-channels-schema", "comm_channel_fanin")
			!= configuration::OK)
	{
		cerr << argv[0] << ": comm-channels-schema required" << endl;
		return 1;
	}

	PostgreSQLConnection pg_conn;
	string str;
	configuration::error status;
	status = conf.get(str, "host", "postgresql");
	if (status == configuration::OK) {
		pg_conn.host(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.host" << endl;
		return 1;
	}
	status = conf.get(str, "hostaddr", "postgresql");
	if (status == configuration::OK) {
		pg_conn.hostaddr(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.hostaddr" << endl;
		return 1;
	}
	status = conf.get(str, "port", "postgresql");
	if (status == configuration::OK) {
		pg_conn.port(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.port" << endl;
		return 1;
	}
	status = conf.get(str, "dbname", "postgresql");
	if (status == configuration::OK) {
		pg_conn.dbname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.dbname" << endl;
		return 1;
	}
	status = conf.get(str, "user", "postgresql");
	if (status == configuration::OK) {
		pg_conn.user(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.user" << endl;
		return 1;
	}
	status = conf.get(str, "password", "postgresql");
	if (status == configuration::OK) {
		pg_conn.password(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.password" << endl;
		return 1;
	}
	status = conf.get(str, "connect-timeout", "postgresql");
	if (status == configuration::OK) {
		pg_conn.connect_timeout(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.connect-timeout" << endl;
		return 1;
	}
	status = conf.get(str, "options", "postgresql");
	if (status == configuration::OK) {
		pg_conn.options(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.options" << endl;
		return 1;
	}
	status = conf.get(str, "sslmode", "postgresql");
	if (status == configuration::OK) {
		pg_conn.sslmode(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.sslmode" << endl;
		return 1;
	}
	status = conf.get(str, "requiressl", "postgresql");
	if (status == configuration::OK) {
		pg_conn.requiressl(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.requiressl" << endl;
		return 1;
	}
	status = conf.get(str, "krbsrvname", "postgresql");
	if (status == configuration::OK) {
		pg_conn.krbsrvname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.krbsrvname" << endl;
		return 1;
	}
	status = conf.get(str, "gsslib", "postgresql");
	if (status == configuration::OK) {
		pg_conn.gsslib(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.gsslib" << endl;
		return 1;
	}
	status = conf.get(str, "service", "postgresql");
	if (status == configuration::OK) {
		pg_conn.service(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.service" << endl;
		return 1;
	}

	if (!pg_conn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pg_conn.error() << endl;
		return 1;
	}

	cerr << "Config file:            " << vm["config-file"].as<string>()
		 << endl;
	cerr << "Date:                   " << date << endl;

	string query("SELECT DISTINCT \"externalIP\" FROM \"" +
				 comm_channels_schema + "\".\"" + date + "\";");

	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	size_t row_count(PQntuples(result));
	if (row_count == 0) {
		cerr << argv[0] << ": No comm channels data for " << date << endl;
		return 0;
	}

	unordered_set<uint32_t> comm_channel_ips;
	for (size_t i(0); i < row_count; ++i) {
		comm_channel_ips.insert(
			lexical_cast<uint32_t>(PQgetvalue(result, i, 0)));
	}
	PQclear(result);

	cerr << "Total External Comm Channel IPs: " << row_count << endl;


	query = "SELECT external_ip, internal_host_count FROM \"" +
			external_contacts_schema + "\".\"" + date + "\";";

	result = PQexec(pg_conn.connection(), query.c_str());
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return 1;
	}

	row_count = PQntuples(result);
	if (row_count == 0) {
		cerr << argv[0] << ": unable to obtain external host fanins!" << endl;
		return 1;
	}

	cerr << "row_count: " << row_count << endl;

	uint32_t host, fanin;
	PGresult *update_result;
	for (size_t i(0); i < row_count; ++i) {
		host = lexical_cast<uint32_t>(PQgetvalue(result, i, 0));

		if (comm_channel_ips.find(host) == comm_channel_ips.end()) {
			continue;
		}

		fanin = lexical_cast<uint32_t>(PQgetvalue(result, i, 1));

		query = "UPDATE \"" + comm_channels_schema + "\".\"" + date +
				"\" SET external_ip_fanin = '" + lexical_cast<string>(fanin) +
				"' WHERE \"externalIP\" = '" + lexical_cast<string>(host) + "';";

		update_result = PQexec(pg_conn.connection(), query.c_str());
		if (result == NULL || PQresultStatus(update_result) != PGRES_COMMAND_OK) {
			cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
			return 1;
		}
		PQclear(update_result);
	}
	PQclear(result);

	return 0;
}
