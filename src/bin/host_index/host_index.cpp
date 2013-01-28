#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "configuration.hpp"
#include "stringHelpers.h"
#include "PostgreSQLConnection.hpp"

using namespace std;
using boost::lexical_cast;
using namespace boost::program_options;

uint32_t get_host_index(vector<uint32_t> &host_index,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn);

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("output-file,o", value<string>(), "the file to which to write the host index")
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

	if (!vm.count("date")) {
		cerr << "Error: date is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	string date(vm["date"].as<string>());

	if (!vm.count("output-file")) {
		cerr << "Error: output-file is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	string output_file(vm["output-file"].as<string>());


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

	string host_index_schema;
	if (conf.get(host_index_schema, "host-index-schema", "host_index")
			!= configuration::OK)
	{
		cerr << argv[0] << ": host-index-schema required" << endl;
		return 1;
	}

	string stats_schema;
	if (conf.get(stats_schema, "stats-schema", "host_index")
			!= configuration::OK)
	{
		cerr << argv[0] << ": stats-schema required" << endl;
		return 1;
	}

	string network_stats_table;
	if (conf.get(network_stats_table, "network-stats-table", "host_index")
			!= configuration::OK)
	{
		cerr << argv[0] << ": network-stats-table required" << endl;
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
	cerr << "host_index_schema:      " << host_index_schema << endl;
	cerr << "stats_schema:           " << stats_schema << endl;
	cerr << "network_stats_table:    " << network_stats_table << endl << endl;
	cerr << "Date:                   " << date << endl;

	string query("SELECT internal_ip_count, external_ip_count FROM \"" +
				 stats_schema + "\".\"" + network_stats_table + "\" "
				 "WHERE date = '" + date + "';");

	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	size_t row_count(PQntuples(result));
	if (row_count == 0) {
		cerr << argv[0] << ": No network-stats data for " << date << endl;
		return 1;
	}
	if (row_count > 1) {
		cerr << argv[0] << ": Duplicate network-stats data for " << date << endl;
		return 1;
	}
	uint32_t internal_ip_count(lexical_cast<uint32_t>(PQgetvalue(result, 0, 0)));
	uint32_t external_ip_count(lexical_cast<uint32_t>(PQgetvalue(result, 0, 1)));
	PQclear(result);

	cerr << "Internal IPs: " << internal_ip_count << endl;
	cerr << "External IPs: " << external_ip_count << endl;
	cerr << "Total IPs:    " << internal_ip_count + external_ip_count << endl;

	vector<uint32_t> host_index(internal_ip_count + external_ip_count);

	if (!get_host_index(host_index,
						host_index_schema,
						date,
						pg_conn))
	{
		cerr << argv[0] << "Unable to retrieve host index from postgres."
			 << endl;
		return 1;
	}

	{
		ofstream ofs(output_file.c_str());
		{
			boost::archive::binary_oarchive oa(ofs);

			oa << host_index;
		}
	}

	return 0;
}

uint32_t get_host_index(vector<uint32_t> &host_index,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn)
{
	string query("SELECT host FROM \"" + host_index_schema + "\".\"" + date +
				 "\" ORDER BY id ASC;");

	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	size_t row_count(PQntuples(result));
	if (row_count != host_index.size()) {
		cerr << "Error: host index size must equal number of entries in pgsql."
			 << endl;
		return false;
	}

	for (size_t i(0); i < row_count; ++i) {
		host_index.at(i) = lexical_cast<uint32_t>(PQgetvalue(result, i, 0));
	}
	PQclear(result);

	return true;
}
