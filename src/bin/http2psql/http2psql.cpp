#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "configuration.hpp"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "StrftimeReadEnumerator.hpp"
//#include "PostgreSQLWriter.hpp"
#include "PostgreSQLBulkWriter.hpp"

using namespace std;
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

int main(int argc, char **argv) {
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

	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).run(), vm);
	}
	catch (error e) {
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

	struct tm _tm;
	string date, end_date;
	date.resize(10);
	end_date.resize(10);
	assert(date.data() != end_date.data());
	time_t tmp_t(time_begin.seconds());
	localtime_r(&tmp_t, &_tm);
	strftime(const_cast<char *>(date.data()), 10, "%Y-%m-%d", &_tm);
	tmp_t = time_end.seconds() - 1;
	localtime_r(&tmp_t, &_tm);
	strftime(const_cast<char *>(end_date.data()), 10, "%Y-%m-%d", &_tm);

	cerr << "Config file: " << vm["config-file"].as<string>().c_str() << endl;
	cerr << "Start time:  " << vm["start-time"].as<TimeStamp>().seconds() << endl;
	cerr << "End time:    " << vm["end-time"].as<TimeStamp>().seconds() << endl;
	cerr << "Table Name:  " << date << endl;
	cerr << "End date:    " << end_date << endl;
	

	if (date != end_date) {
		cerr << "Error: the interval must not span more than one day, localtime" << endl;
		return 1;
	}
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

	string data_directory;
	if (conf.get(data_directory, "data-directory", "http2psql", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string request_schema;
	if (conf.get(request_schema, "request-schema", "http2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": request-schema required" << endl;
		return 1;
	}

	string response_schema;
	if (conf.get(response_schema, "response-schema", "http2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": response-schema required" << endl;
		return 1;
	}
	size_t buffer_count;
	if (conf.get(buffer_count, "buffer-count", "http2psql")
			!= configuration::OK)
	{
		cerr << argv[0] << ": buffer-count required" << endl;
		return 1;
	}
	boost::shared_ptr<StrftimeReadEnumerator> request_enum(
		new StrftimeReadEnumerator);
	cerr << "derrick[1.2.1]" << endl;
	request_enum->init(data_directory,
				   "%Y/%m/%d/http_request_%H",
				   time_begin,
				   time_end);
	if (!(*request_enum)) {
		cerr << "Error: Unable to initialize read enumerator" << endl;
		return 1;
	}

	EnumeratedFileReader
		<FlatFileReader
			<HTTPRequest>,
		 StrftimeReadEnumerator
		> request_reader;
	
	ErrorStatus error_status;
	error_status = request_reader.init(request_enum);
	if (error_status != E_SUCCESS && error_status != E_EOF) {
		cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
		return 1;
	}

	PostgreSQLBulkWriter<HTTPRequest> request_writer(pg_conn,
													 request_schema,
													 date);

	if (!request_writer) {
		cerr << argv[0] << ": request_writer initialization failed: " << endl;
		cerr << "\t" << request_writer.error() << endl;
		return 1;
	}
		cerr << "derrick[4]" << endl;

	HTTPRequest http_request;
	ErrorStatus reader_status;
	size_t rows(0);
	while ((reader_status = request_reader.read(http_request)) == E_SUCCESS) {
//		cerr << "#derrick: write http request " << rows << "rows" << endl;
		if (!request_writer.write(http_request)) {
			cerr << argv[0] << ": unable to write request!" << endl;
			return 1;
		}
		++rows;
		if (rows % buffer_count == 0) {
			if (!request_writer.flush()) {
				cerr << argv[0] << ": request_writer.flush() failed!" << endl;
				return 1;
			}
		}
	}
	if (reader_status != E_EOF) {
		cerr << argv[0] << ": error reading requests!" << endl;
		return 1;
	}
	cerr << "Requests read: " << rows << endl;
	if (!request_writer.close()) {
		cerr << argv[0] << ": request_writer.close() failed: " << endl;
		cerr << "\t" << request_writer.error() << endl;
		return 1;
	}

	boost::shared_ptr<StrftimeReadEnumerator> response_enum(
		new StrftimeReadEnumerator);

		cerr << "derrick[5]" << endl;

	response_enum->init(data_directory,
				   "%Y/%m/%d/http_response_%H",
				   time_begin,
				   time_end);
	if (!(*response_enum)) {
		cerr << "Error: Unable to initialize read enumerator" << endl;
		return 1;
	}

	EnumeratedFileReader
		<FlatFileReader
			<HTTPResponse>,
		 StrftimeReadEnumerator
		> response_reader;
	error_status = response_reader.init(response_enum);
	if (error_status != E_SUCCESS && error_status != E_EOF) {
		cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
		return 1;
	}

	PostgreSQLBulkWriter<HTTPResponse> response_writer(pg_conn,
												   response_schema,
												   date);

	cerr << "Inserting responses..." << endl;
	HTTPResponse http_response;
	rows = 0;
	while ((reader_status = response_reader.read(http_response)) == E_SUCCESS) {
		if (!response_writer.write(http_response)) {
			cerr << argv[0] << ": unable to write response!" << endl;
			return 1;
		}
		++rows;
		if (rows % buffer_count == 0) {
			if (!response_writer.flush()) {
				cerr << argv[0] << ": response_writer.flush() failed!" << endl;
				return 1;
			}
		}
	}
	if (reader_status != E_EOF) {
		cerr << argv[0] << ": error reading responses!" << endl;
		return 1;
	}
	if (!response_writer.flush()) {
		cerr << argv[0] << ": response_writer.flush() failed!" << endl;
		return 1;
	}
	cerr << "Responses inserted: " << rows << endl;
}
