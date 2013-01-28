#include <iostream>
#include <string>
#include <vector>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <tr1/functional_hash.h>

#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/bimap.hpp>

#include "configuration.hpp"
#include "IPv4Network.hpp"
#include "stringHelpers.h"
#include "PostgreSQLConnection.hpp"
#include "PostgreSQLBulkWriter.hpp"
#include "PostgreSQLWriter.hpp"
#include "host_ip_index.hpp"
#include "host_index_pair.hpp"
#include "network_stats.hpp"

#include "hostPair.hpp"
#include "address.h"

namespace std {
namespace tr1 {

template<>
struct hash<pair<uint32_t, uint32_t> >
{
	std::tr1::hash<uint64_t> uint64_hasher;

	std::size_t operator()(const std::pair<uint32_t, uint32_t> &p) const {
		return uint64_hasher(*(reinterpret_cast<const uint64_t *>(&p)));
	}
};

}
}

using namespace std;
using namespace tr1;
using namespace boost;
using namespace boost::program_options;
using namespace boost::asio;

typedef pair<uint32_t, uint32_t> host_pair;
typedef std::tr1::hash<host_pair> host_pair_hash;

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

	if (!vm.count("date")) {
		cerr << "Error: date is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	string date(vm["date"].as<string>());


	struct tm _tm;
	if (strptime(date.c_str(), "%Y-%m-%d", &_tm) == NULL) {
		cerr << "Error: invalid date: " << vm["date"].as<string>() << endl;
		return 1;
	}

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string local_networks_string;
	if (conf.get(local_networks_string, "local-networks", "index_host_pair_ips", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid local-networks" << endl;
		return 1;
	}
	std::vector <std::pair <uint32_t, uint32_t> > local_networks;
	getLocalNetworks(local_networks, local_networks_string);

	string hostpairs_schema;
	if (conf.get(hostpairs_schema, "hostpairs-schema", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": hostpairs-schema required" << endl;
		return 1;
	}

	string host_index_schema;
	if (conf.get(host_index_schema, "host-index-schema", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": host-index-schema required" << endl;
		return 1;
	}

	string host_index_pair_schema;
	if (conf.get(host_index_pair_schema, "host-index-pair-schema", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": host-index-pair-schema required" << endl;
		return 1;
	}

	string stats_schema;
	if (conf.get(stats_schema, "stats-schema", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": stats-schema required" << endl;
		return 1;
	}

	string network_stats_table;
	if (conf.get(network_stats_table, "network-stats-table", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": network-stats-table required" << endl;
		return 1;
	}

	size_t buffer_count;
	if (conf.get(buffer_count, "buffer-count", "index_host_pair_ips")
			!= configuration::OK)
	{
		cerr << argv[0] << ": buffer-count required" << endl;
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

	cerr << "Date:                   " << date << endl;
	cerr << "Config file:            " << vm["config-file"].as<string>() << endl;
	cerr << "hostpairs_schema:       " << hostpairs_schema << endl;
	cerr << "host_index_schema:      " << host_index_schema << endl;
	cerr << "host_index_pair_schema: " << host_index_pair_schema << endl;
	cerr << "stats_schema:           " << stats_schema << endl;
	cerr << "network_stats_table:    " << network_stats_table << endl;
	cerr << "buffer_count:           " << buffer_count << endl;

	unordered_map<uint32_t, uint32_t> internal_ip_index;
	unordered_map<uint32_t, uint32_t> external_ip_index;
	unordered_set<host_pair, host_pair_hash > host_pairs;

	string query("SELECT * from \"" + hostpairs_schema + "\".\"" + date + "\";");
	
	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	size_t row_count(PQntuples(result));
	uint32_t internal_ip, external_ip;
	string ip_string;
	vector<string> ip_strings;
	//cout << "DEBUG: result row_count: " << row_count << endl;
	for (size_t i(0); i < row_count; ++i) {
		/*
		if (i % 100 == 0) {
			cerr << "DEBUG: reading row " << i << "/" << row_count << endl;
		}
		*/
		internal_ip = lexical_cast<uint32_t>(PQgetvalue(result, i, 0));

		if (internal_ip_index.find(internal_ip) == internal_ip_index.end()) {
			internal_ip_index.insert(std::pair<uint32_t, uint32_t>(
										internal_ip,
										internal_ip_index.size()));
		}

		ip_string = PQgetvalue(result, i, 1);
		ip_string = ip_string.substr(1, ip_string.length() - 2);
		ip_strings.clear();
		explodeString(ip_strings,
					  ip_string,
					  ",");
		for (vector<string>::const_iterator str(ip_strings.begin());
			 str != ip_strings.end();
			 ++str)
		{
			external_ip = lexical_cast<uint32_t>(*str);
			if (isInternal(external_ip, local_networks)) {
				continue;
			}

			if (external_ip_index.find(external_ip) == external_ip_index.end())
			{
				external_ip_index.insert(std::pair<uint32_t, uint32_t>(
											external_ip,
											external_ip_index.size()));
			}

			host_pairs.insert(make_pair(internal_ip, external_ip));
		}
	}
	PQclear(result);

	PostgreSQLBulkWriter<host_ip_index> index_writer(pg_conn,
													 host_index_schema,
													 date);
	
	if (!index_writer) {
		cerr << argv[0] << ": index_writer initialization failed: " << endl;
		cerr << "\t" << index_writer.error() << endl;
		return 1;
	}

	size_t rows(0);
	for (unordered_map<uint32_t, uint32_t>::iterator i(
			internal_ip_index.begin());
		 i != internal_ip_index.end();
		 ++i)
	{
		if (!index_writer.write(host_ip_index(i->first, i->second))) {
			cerr << argv[0] << ": unable to write internal host index!" << endl;
			return 1;
		}
		++rows;
		if (rows % buffer_count == 0) {
			if (!index_writer.flush()) {
				cerr << argv[0] << ": index_writer.flush() failed!" << endl;
				return 1;
			}
		}
	}
	cerr << "internal host indexes written:  " << rows << endl;
	size_t internal_rows(rows);

	for (unordered_map<uint32_t, uint32_t>::iterator i(
			external_ip_index.begin());
		 i != external_ip_index.end();
		 ++i)
	{
		if (!index_writer.write(
				host_ip_index(i->first,
							  internal_ip_index.size() + i->second)))
		{
			cerr << argv[0] << ": unable to write external host index!" << endl;
			return 1;
		}
		++rows;
		if (rows % buffer_count == 0) {
			if (!index_writer.flush()) {
				cerr << argv[0] << ": index_writer.flush() failed!" << endl;
				return 1;
			}
		}
	}
	size_t external_rows(rows - internal_rows);
	cerr << "external host indexes written:  " << external_rows << endl;

	if (!index_writer.close()) {
		cerr << argv[0] << ": index_writer.close() failed: " << endl;
		cerr << "\t" << index_writer.error() << endl;
		return 1;
	}

	query = ("ALTER TABLE \"" + host_index_schema + "\".\"" + date + "\" ADD PRIMARY KEY(host);");
	result = PQexec(pg_conn.connection(), query.c_str());
	if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}
	PQclear(result);

	query = ("ALTER TABLE \"" + host_index_schema + "\".\"" + date + "\" ADD UNIQUE(id);");
	result = PQexec(pg_conn.connection(), query.c_str());
	if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}
	PQclear(result);

	cerr << "total host indexes written:     " << rows << endl;


	PostgreSQLBulkWriter<host_index_pair> index_pair_writer(
														pg_conn,
														host_index_pair_schema,
														date);
	
	if (!index_pair_writer) {
		cerr << argv[0] << ": index_pair_writer initialization failed: "
			 << endl;
		cerr << "\t" << index_pair_writer.error() << endl;
		return 1;
	}

	rows = 0;
	for (unordered_set<host_pair, host_pair_hash >::const_iterator i(
			host_pairs.begin());
		 i != host_pairs.end();
		 ++i)
	{
		if (!index_pair_writer.write(
				host_index_pair(
					internal_ip_index[i->first],
					external_ip_index[i->second] + internal_ip_index.size())))
		{
			cerr << argv[0] << ": unable to write host index pair!" << endl;
			return 1;
		}

		++rows;
		if (rows % buffer_count == 0) {
			if (!index_pair_writer.flush()) {
				cerr << argv[0] << ": index_pair_writer.flush() failed!"
					 << endl;
				return 1;
			}
		}
	}
	if (!index_pair_writer.close()) {
		cerr << argv[0] << ": index_pair_writer.close() failed: " << endl;
		cerr << "\t" << index_pair_writer.error() << endl;
		return 1;
	}

	cerr << "total host index pairs written: " << rows << endl;

	network_stats net_stats(date, internal_rows, external_rows);
	PostgreSQLWriter<network_stats> stats_writer(pg_conn,
												 stats_schema,
												 network_stats_table);

	if (!stats_writer) {
		cerr << argv[0] << ": stats_writer initialization failed: "
			 << endl;
		cerr << "\t" << stats_writer.error() << endl;
		return 1;
	}

	if (!stats_writer.write(net_stats)) {
		cerr << argv[0] << ": unable to write network stats!" << endl;
		return 1;
	}
}
