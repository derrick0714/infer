#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/bimap.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "configuration.hpp"
#include "IPv4Network.hpp"
#include "stringHelpers.h"
#include "PostgreSQLConnection.hpp"
#include "all_simple_paths_visitor.hpp"

using namespace std;
using namespace tr1;
using namespace boost;
using namespace boost::program_options;
using namespace boost::asio;

typedef adjacency_list <vecS, vecS, undirectedS> UndirectedGraph;

void validate(boost::any &v,
			  const std::vector<std::string> &values,
			  IPv4Network *, int)
{
	using namespace boost::program_options;
	using namespace boost::asio::ip;

	validators::check_first_occurrence(v);

	const std::string &s = validators::get_single_string(values);

	string::size_type pos(s.find('/'));

	address_v4 ip;
	try {
		ip = address_v4::from_string(s.substr(0,pos));
	} catch (const boost::system::system_error &e) {
		//throw validation_error("invalid CIDR block: bad IP address");
		throw validation_error(validation_error::invalid_option_value);
	}

	uint16_t mask(32);
	if (pos != string::npos) {
		// get mask
		try {
			mask = boost::lexical_cast<uint16_t>(s.substr(pos+1));
		} catch (const boost::bad_lexical_cast &e) {
			//throw validation_error("invalid CIDR block: bad netmask");
			throw validation_error(validation_error::invalid_option_value);
		}

		if (mask == 0 || mask > 32) {
			//throw validation_error("invalid CIDR block: netmask out of range");
			throw validation_error(validation_error::invalid_option_value);
		}
	}

	IPv4Network net;
	if (!net.set(ip.to_ulong(), 0xffffffff << (32 - mask))) {
		//throw validation_error("invalid CIDR block: network/netmask mismatch");
		throw validation_error(validation_error::invalid_option_value);
	}

	v = boost::any(net);
}

/*
bool populate_privacy_graph(boost::numeric::ublas::matrix<double>
								&privacy_graph,
							 uint32_t ip,
							 unordered_set<uint32_t> &external_ip_set,
							 unordered_map<uint32_t, size_t>
								&internal_ip_index,
							 PostgreSQLConnection &pg_conn,
							 const string &date,
							 size_t privacy_threshold);
*/
uint32_t get_index_from_host(uint32_t host,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn);

uint32_t get_host_from_index(uint32_t index,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn);

bool populate_privacy_graph(boost::numeric::ublas::matrix<double>
									&privacy_graph,
							  UndirectedGraph &network_graph,
							  uint32_t internal_ip_count,
							  uint32_t external_ip_count,
							  size_t privacy_threshold);

bool populate_network_graph(UndirectedGraph &network_graph,
							uint32_t internal_ip_count,
							uint32_t external_ip_count,
							const string &host_index_pair_schema,
							const string &date,
							PostgreSQLConnection &pg_conn);

void prune_network_graph(UndirectedGraph &network_graph,
						 uint32_t internal_ip_count,
						 uint32_t external_ip_count,
						 size_t privacy_threshold);

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("input-file,i", value<string>(), "the file from which to read the privacy graph")
		("host-index-file,x", value<string>(), "the file from which to read the host index")
		("decimal", "output IP addresses in decimal form")
		("date", value<string>(), "the date (YYYY-mm-dd)")
		("host", value<IPv4Network>(), "the \"seed\" host")
	;

	positional_options_description p;
	p.add("date", 1).add("host", 1);
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

	string input_file;
	if (vm.count("input-file")) {
		input_file.assign(vm["input-file"].as<string>());
	}

	string host_index_file;
	if (vm.count("host-index-file")) {
		host_index_file.assign(vm["host-index-file"].as<string>());
	}

	if (!vm.count("host")) {
		cerr << "Error: host is required." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	uint32_t host(vm["host"].as<IPv4Network>().network());

	bool decimal(false);
	if (vm.count("decimal")) {
		decimal = true;
	}


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
	if (conf.get(host_index_schema, "host-index-schema", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": host-index-schema required" << endl;
		return 1;
	}

	string host_index_pair_schema;
	if (conf.get(host_index_pair_schema, "host-index-pair-schema", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": host-index-pair-schema required" << endl;
		return 1;
	}

	string stats_schema;
	if (conf.get(stats_schema, "stats-schema", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": stats-schema required" << endl;
		return 1;
	}

	string network_stats_table;
	if (conf.get(network_stats_table, "network-stats-table", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": network-stats-table required" << endl;
		return 1;
	}

	uint32_t privacy_threshold;
	if (conf.get(privacy_threshold, "privacy-threshold", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": privacy-threshold required" << endl;
		return 1;
	}

	uint32_t dye_iterations;
	if (conf.get(dye_iterations, "dye-iterations", "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": dye-iterations required" << endl;
		return 1;
	}

	double confidence_threshold;
	if (conf.get(confidence_threshold,
				 "confidence-threshold",
				 "related_hosts_graph")
			!= configuration::OK)
	{
		cerr << argv[0] << ": confidence-threshold required" << endl;
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
	cerr << "host_index_pair_schema: " << host_index_pair_schema << endl;
	cerr << "stats_schema:           " << stats_schema << endl;
	cerr << "network_stats_table:    " << network_stats_table << endl << endl;
	cerr << "Date:                   " << date << endl;
	cerr << "Host:                   " << ip::address_v4(host) << endl;
	cerr << "privacy-threshhold:     " << privacy_threshold << endl;
	cerr << "dye-iterations:         " << dye_iterations << endl;
	cerr << "confidence-threshold:   " << confidence_threshold << endl;

	uint32_t host_id;
	if (host == 0) {
		host_id = numeric_limits<uint32_t>::max();
	}
	else {
		host_id = get_index_from_host(host,
										 host_index_schema,
										 date,
										 pg_conn);
		if (host_id == numeric_limits<uint32_t>::max()) {
			cerr << argv[0] << ": host not seen on " << date << endl;
			return 1;
		}
	}

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

	if (host != 0 && host_id >= internal_ip_count)
	{
		cerr << argv[0] << ": host must be internal" << endl;
		return 1;
	}

	UndirectedGraph network_graph;
	if (input_file.empty()) {
		if (!populate_network_graph(network_graph,
									internal_ip_count,
									external_ip_count,
									host_index_pair_schema,
									date,
									pg_conn))
		{
			cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
			return 1;
		}

		cerr << "Network graph populated." << endl;

		prune_network_graph(network_graph,
							internal_ip_count,
							external_ip_count,
							privacy_threshold);
		cerr << "Network graph pruned..." << endl;
	}
	else {
		// only do this if input file is specified...
		ifstream ifs(input_file.c_str());
		{
			boost::archive::binary_iarchive ia(ifs);

			ia >> network_graph;
		}
	}

	vector<uint32_t> host_index;
	{
		ifstream ifs(host_index_file.c_str());
		{
			boost::archive::binary_iarchive ia(ifs);

			ia >> host_index;
		}
	}

	boost::numeric::ublas::matrix<double> privacy_graph;
	if (!populate_privacy_graph(privacy_graph,
								  network_graph,
								  internal_ip_count,
								  external_ip_count,
								  privacy_threshold))
	{
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return 1;
	}

	cerr << "Privacy graph populated." << endl;

	//cout << "DEBUG:" << endl << privacy_graph << endl;
	// normalize the contacts graph

	double sum;
	for (size_t i(0); i < privacy_graph.size2(); ++i) {
		using namespace boost::numeric;
		ublas::matrix_column<ublas::matrix<double> >
			col(privacy_graph, i);
		sum = boost::numeric::ublas::sum(col);
		if (sum == 0) {
			continue;
		}
		for (size_t j(0); j < col.size(); ++j) {
			col(j) /= sum;
		}
	}

	uint32_t index_begin, index_end;
	if (host == 0) {
		index_begin = 0;
		index_end = internal_ip_count;
	}
	else {
		index_begin = host_id;
		index_end = index_begin + 1;
	}

	for (uint32_t id(index_begin); id < index_end; ++id) {
		if (!host_index_file.empty()) {
			host = host_index.at(id);
		}
		else {
			host = get_host_from_index(id,
									   host_index_schema,
									   date,
									   pg_conn);
		}

		boost::numeric::ublas::vector<double> d(
			boost::numeric::ublas::zero_vector<double>(privacy_graph.size1()));

		for (size_t i(0); i < dye_iterations; ++i) {
			d(id) = 1;
			sum = boost::numeric::ublas::sum(d);
			for (size_t j(0); j < d.size(); ++j) {
				d[j] /= sum;
			}
			d = boost::numeric::ublas::prod(privacy_graph, d);
		}

		map<double, uint32_t> results;
		for (size_t i(0); i < internal_ip_count; ++i) {
			if (d(i) > confidence_threshold) {
				results.insert(make_pair(d(i), i)); 
			}
		}

		uint32_t tmp_ip;
		for (map<double, uint32_t>::const_iterator i(results.begin());
			 i != results.end();
			 ++i)
		{
			if (!host_index_file.empty()) {
				tmp_ip = host_index.at(i->second);
			}
			else {
				tmp_ip = get_host_from_index(i->second,
											 host_index_schema,
											 date,
											 pg_conn);
			}
			if (decimal) {
				//cout << host << '\t' << i->second << '\t' << i->first << endl;
				cout << host << '\t' << tmp_ip << '\t' << i->first << endl;
			}
			else {
				//cout << ip::address_v4(host) << '\t' << ip::address_v4(i->second) << '\t' << i->first << endl;
				cout << ip::address_v4(host) << '\t' << ip::address_v4(tmp_ip) << '\t' << i->first << endl;
			}
		}
		{
			/*
			for each internal host we're interested in
				initiate a 2 level depth-limited search for the other ones
			*/

			// returns the second element
			typedef map<double, uint32_t>::value_type value_type;
			boost::function<int(value_type&)> f = boost::bind(&value_type::second, _1);

			cerr << "num_vertices(): " << num_vertices(network_graph) << endl;

			all_simple_paths_visitor<UndirectedGraph>
				vis(network_graph,
					boost::make_transform_iterator(results.begin(), f),
					boost::make_transform_iterator(results.end(), f));

			depth_first_visit(network_graph,
							  id,
							  vis,
							  vis.color_map(),
							  vis.terminator_function());
			for (map<double, uint32_t>::const_iterator i(results.begin());
				 i != results.end();
				 ++i)
			{
				depth_first_visit(network_graph,
								  i->second,
								  vis,
								  vis.color_map(),
								  vis.terminator_function());
			}

			UndirectedGraph sub(vis.create_subgraph());
			graph_traits<UndirectedGraph>::vertex_iterator vi, vi_end;
			graph_traits<UndirectedGraph>::out_edge_iterator out, out_end;
			for (tie(vi, vi_end) = vertices(sub); vi != vi_end; ++vi) {
				if (!host_index_file.empty()) {
					tmp_ip = host_index.at(vis.local_to_global(*vi));
				}
				else {
					tmp_ip = get_host_from_index(vis.local_to_global(*vi),
												 host_index_schema,
												 date,
												 pg_conn);
				}
				if (decimal) {
					cout << tmp_ip << " <--> ";
				}
				else {
					cout << ip::address_v4(tmp_ip) << " <--> ";
				}
				for (tie(out, out_end) = out_edges(*vi, sub);
					 out != out_end;
					 ++out)
				{
					if (!host_index_file.empty()) {
						tmp_ip = host_index.at(vis.local_to_global(
												target(*out, sub)));
					}
					else {
						tmp_ip = get_host_from_index(vis.local_to_global(
														target(*out, sub)),
													 host_index_schema,
													 date,
													 pg_conn);
					}
					if (decimal) {
						cout << tmp_ip << " ";
					}
					else {
						cout << ip::address_v4(tmp_ip) << " ";
					}
				}
				cout << endl;
			}
		}
	}

	return 0;
}

bool populate_network_graph(UndirectedGraph &network_graph,
							uint32_t internal_ip_count,
							uint32_t external_ip_count,
							const string &host_index_pair_schema,
							const string &date,
							PostgreSQLConnection &pg_conn)
{
	network_graph = UndirectedGraph(internal_ip_count + external_ip_count);

	cerr << "Network graph initialized" << endl;

	string query("SELECT * FROM \"" + host_index_pair_schema + "\".\"" + date + "\";");
	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	uint32_t internal_id;
	uint32_t external_id;
	size_t row_count(PQntuples(result));
	for (size_t i(0); i < row_count; ++i) {
		/*
		if ((i + 1) % 100000 == 0) {
			cerr << "DEBUG: " << i+1 << '/' << row_count << endl;
		}
		*/
		internal_id = lexical_cast<uint32_t>(PQgetvalue(result, i, 0));
		external_id = lexical_cast<uint32_t>(PQgetvalue(result, i, 1));

		add_edge(vertex(internal_id, network_graph),
				 vertex(external_id, network_graph),
				 network_graph);
	}
	PQclear(result);

	return true;
}

bool populate_privacy_graph(boost::numeric::ublas::matrix<double>
									&privacy_graph,
							  UndirectedGraph &network_graph,
							  uint32_t internal_ip_count,
							  uint32_t external_ip_count,
							  size_t privacy_threshold)
{
	privacy_graph =
		boost::numeric::ublas::zero_matrix<double>(internal_ip_count,
												   internal_ip_count);

	graph_traits<UndirectedGraph>::out_edge_iterator out, out_end;
	size_t total_ip_count(internal_ip_count + external_ip_count);
	vector<size_t> hosts;
	for (size_t i(internal_ip_count); i < total_ip_count; ++i) {
		if (out_degree(vertex(i, network_graph), network_graph)
				<= privacy_threshold)
		{
			for (tie(out, out_end) = out_edges(vertex(i, network_graph),
											   network_graph);
				 out != out_end;
				 ++out)
			{
				//cout << *out << endl;
				hosts.push_back(target(*out, network_graph));
			}

			for (vector<size_t>::const_iterator j(hosts.begin());
				 j != hosts.end();
				 ++j)
			{
				++(privacy_graph(*j, *j));

				vector<size_t>::const_iterator k(j);
				++k;
				for (;
					 k < hosts.end();
					 ++k)
				{
					++(privacy_graph(*j, *k));
					++(privacy_graph(*k, *j));
				}
			}
			hosts.clear();
		}
	}

	return true;
}

void prune_network_graph(UndirectedGraph &network_graph,
						 uint32_t internal_ip_count,
						 uint32_t external_ip_count,
						 size_t privacy_threshold)
{
	size_t total_ip_count(internal_ip_count + external_ip_count);

	for (size_t i(internal_ip_count); i < total_ip_count; ++i) {
		if (out_degree(vertex(i, network_graph), network_graph)
				> privacy_threshold)
		{
			//--external_ip_count;
			//--total_ip_count;
			// TODO is this method only correct for adjacency list?
			// NOTE! this does not remove vertices, just edges...
			clear_vertex(i, network_graph);
			//remove_vertex(i, network_graph);
			//--i;
		}
	}
}

uint32_t get_index_from_host(uint32_t host,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn)
{
	string query("SELECT id FROM \"" + host_index_schema + "\".\"" + date +
				 "\" WHERE host = '" + lexical_cast<string>(host) + "';");

	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return numeric_limits<uint32_t>::max();
	}

	size_t row_count(PQntuples(result));
	if (row_count == 0) {
		return numeric_limits<uint32_t>::max();
	}

	uint32_t ret(lexical_cast<uint32_t>(PQgetvalue(result, 0, 0)));

	PQclear(result);
	return ret;
}

uint32_t get_host_from_index(uint32_t index,
							 const string &host_index_schema,
							 const string &date,
							 PostgreSQLConnection &pg_conn)
{
	string query("SELECT host FROM \"" + host_index_schema + "\".\"" + date +
				 "\" WHERE id = '" + lexical_cast<string>(index) + "';");

	PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
	if (result == NULL || PQresultStatus(result) != PGRES_TUPLES_OK) {
		cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
		return false;
	}

	size_t row_count(PQntuples(result));
	if (row_count == 0) {
		cerr << "Warning: No ip found for index \"" << index << "\"." << endl;
		return 0;
	}

	uint32_t ret(lexical_cast<uint32_t>(PQgetvalue(result, 0, 0)));

	PQclear(result);
	return ret;
}
