#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "configuration.hpp"
#include "DNS.hpp"
#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "PostgreSQLBulkWriter.hpp"

#include "dns_row.hpp"
#include "dns_response_row.hpp"

using namespace std;
using namespace boost;
using namespace boost::program_options;

void validate(boost::any &v,
			  const std::vector<std::string> &values,
			  TimeStamp *, int)
{
	using namespace boost::program_options;

	validators::check_first_occurrence(v);

	const std::string &s = validators::get_single_string(values);

	struct tm tmpTime;
	std::string format("%Y-%m-%d %H:%M:%S");

	time_t tmpTimet(0);
	gmtime_r(&tmpTimet, &tmpTime);

	char *ret;
	while (format.size() > 2 &&
		  ((ret = strptime(s.c_str(), format.c_str(), &tmpTime)) == NULL ||
		   (*ret != '\0')))
	{
		format.resize(format.size() - 3);
	}
	if (ret == NULL || *ret != '\0') {
		if ((ret = strptime(s.c_str(), format.c_str(), &tmpTime)) == NULL ||
			(*ret != '\0'))
		{
			//throw validation_error("invalid timestamp");
			throw validation_error(validation_error::invalid_option_value);
		}
	}

	tmpTimet = timegm(&tmpTime);

	v = boost::any(TimeStamp(static_cast <uint32_t>(tmpTimet), 0));
}

int64_t get_next_id(PostgreSQLConnection &pg_conn,
					const string &dns_schema,
					const string &dns_table)
{
	string query(
		"SELECT nextval('\"" + dns_schema + "\".\"" +
		dns_table + "_id_seq\"');");
	PGresult *result(PQexecParams(pg_conn.connection(),
								  query.c_str(),
								  0,
								  NULL,
								  NULL,
								  NULL,
								  NULL,
								  1));
	if (PQresultStatus(result) != PGRES_TUPLES_OK ||
			PQntuples(result) != 1) {
		PQclear(result);
		cerr << PQerrorMessage(pg_conn.connection());
		return -1;
	}

	uint64_t ret(*reinterpret_cast<uint64_t*>(PQgetvalue(result, 0, 0)));
	PQclear(result);
	return static_cast<int64_t>(be64toh(ret));
}

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("start-time", value<TimeStamp>(), "beginning of interval")
		("end-time", value<TimeStamp>(), "end of interval")
	;

	positional_options_description p;
	p.add("start-time", 1).add("end-time", 1);
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

	if (!vm.count("start-time")) {
		cerr << "Error: start-time is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	if (!vm.count("end-time")) {
		cerr << "Error: end-time is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}

	TimeStamp time_begin(vm["start-time"].as<TimeStamp>()),
			  time_end(vm["end-time"].as<TimeStamp>());

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
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

	PostgreSQLConnection response_pg_conn(pg_conn);
	if (!response_pg_conn.open()) {
		cerr << argv[0] << ": unable to open response PostgreSQL connection" << endl
			 << response_pg_conn.error() << endl;
		return 1;
	}

	PostgreSQLConnection id_pg_conn(pg_conn);
	if (!id_pg_conn.open()) {
		cerr << argv[0] << ": unable to open id PostgreSQL connection" << endl
			 << id_pg_conn.error() << endl;
		return 1;
	}


	string data_directory;
	if (conf.get(data_directory, "data-directory", "dns2psql", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string dns_schema;
	if (conf.get(dns_schema, "dns-schema", "dns2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": dns-schema required" << endl;
		return 1;
	}

	string dns_table;
	if (conf.get(dns_table, "dns-table", "dns2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": dns-table required" << endl;
		return 1;
	}

	string dns_response_schema;
	if (conf.get(dns_response_schema, "dns-response-schema", "dns2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": dns-response-schema required" << endl;
		return 1;
	}

	string dns_response_table;
	if (conf.get(dns_response_table, "dns-response-table", "dns2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": dns-response-table required" << endl;
		return 1;
	}

	size_t buffer_count;
	if (conf.get(buffer_count, "buffer-count", "dns2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": buffer-count required" << endl;
		return 1;
	}


	boost::shared_ptr<StrftimeReadEnumerator> dns_enum(
		new StrftimeReadEnumerator);

	dns_enum->init(data_directory,
				   "%Y/%m/%d/dns_%H",
				   time_begin,
				   time_end);
	if (!(*dns_enum)) {
		cerr << "Error: Unable to initialize read enumerator" << endl;
		return 1;
	}

	EnumeratedFileReader
		<FlatFileReader
			<DNS>,
		 StrftimeReadEnumerator
		> dns_reader;
	ErrorStatus error_status;
	error_status = dns_reader.init(dns_enum);
	if (error_status != E_SUCCESS && error_status != E_EOF) {
		cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
		return 1;
	}
	if (error_status == E_EOF) {
		return 0;
	}

	PostgreSQLBulkWriter<dns_row> dns_writer(pg_conn,
											 dns_schema,
											 dns_table);

	if (!dns_writer) {
		cerr << argv[0] << ": dns_writer initialization failed: " << endl;
		cerr << '\t' << dns_writer.error() << endl;
		return 1;
	}

	PostgreSQLBulkWriter<dns_response_row> response_writer(response_pg_conn,
														   dns_response_schema,
														   dns_response_table);

	if (!response_writer) {
		cerr << argv[0] << ": response_writer initialization failed: " << endl;
		cerr << '\t' << response_writer.error() << endl;
		return 1;
	}

	DNS dns;
	dns_row row;
	dns_response_row response_row;
	int64_t next_id;
	size_t rows(0), response_rows(0);
	ErrorStatus reader_status;
	while ((reader_status = dns_reader.read(dns)) == E_SUCCESS) {
		// call psql nextval on sequence
		next_id = get_next_id(id_pg_conn, dns_schema, dns_table);
		if (next_id < 0) {
			cerr << argv[0] << ": unable to get next dns sequence id!" << endl;
			return 1;
		}

		// forumlate the dns row
		row.id = next_id;
		row.query_time = dns.queryTime();
		row.response_time = dns.responseTime();
		row.client_ip = dns.clientIP();
		row.server_ip = dns.serverIP();
		row.query_name = dns.queryName();
		row.query_type = dns.queryType();
		row.response_code = dns.responseFlags() & 0x000F;

		// insert the dns row
		if (!dns_writer.write(row)) {
			cerr << argv[0] << ": unable to write DNS row!" << endl;
			return 1;
		}

		// for each dns response
		const std::vector<DNS::DNSResponse*> &responses(dns.responses());
		for (std::vector<DNS::DNSResponse*>::const_iterator
				response(responses.begin());
			 response != responses.end();
			 ++response)
		{
			// formulate the dns_response_row
			response_row.dns_id = next_id;
			response_row.name = (*response)->name();
			response_row.type = (*response)->type();
			response_row.resource_data = (*response)->resourceData();
			response_row.ttl = (*response)->ttl();

			// insert the row
			if (!response_writer.write(response_row)) {
				cerr << argv[0] << ": unable to write response!" << endl;
				return 1;
			}

			++response_rows;
		}

		if (rows % buffer_count == 0) {
			if (!dns_writer.flush()) {
				cerr << argv[0] << ": dns_writer.flush() failed!" << endl;
				cerr << dns_writer.error() << endl;
				cerr << next_id << endl;
				return 1;
			}
			if (!response_writer.flush()) {
				cerr << argv[0] << ": response_writer.flush() failed!" << endl;
				return 1;
			}
		}

		++rows;
	}
	if (reader_status != E_EOF) {
		cerr << argv[0] << ": error reading DNS records!" << endl;
		return 1;
	}

	cerr << "DNS records inserted:   " << rows << endl;
	cerr << "DNS responses inserted: " << response_rows << endl;

	if (!dns_writer.close()) {
		cerr << argv[0] << ": dns_writer.close() failed: " << endl;
		cerr << "\t" << dns_writer.error() << endl;
		return 1;
	}

	if (!response_writer.close()) {
		cerr << argv[0] << ": response_writer.close() failed: " << endl;
		cerr << "\t" << response_writer.error() << endl;
		return 1;
	}

	return 0;
}
