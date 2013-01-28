#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "configuration.hpp"
#include "HTTPRequest.h"
#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "stringHelpers.h"
#include "address.h"
#include "hostPair.hpp"
#include "PostgreSQLBulkWriter.hpp"
#include "browser_stat.hpp"
#include "browser_version_stat.hpp"

using namespace std;
using boost::regex;
using boost::smatch;
using namespace boost::program_options;

struct request_stat_t {
	set<string> user_agents;
	set<pair<string, string> > browsers;
};

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

bool get_browser_from_user_agent(pair<string, string> &browser,
								 const string &ua)
{
	static const regex
		firefox("Firefox\\/([[:digit:]\\.]+)"),
		msie("MSIE ([[:digit:]\\.]+)"),
		chrome("Chrome\\/([[:digit:]\\.]+)"),
		opera("Opera\\/([[:digit:]\\.]+)"),
		safari_mobile("Version\\/([[:digit:]\\.]+ Mobile).*Safari"),
		safari("Version\\/([[:digit:]\\.]+).*Safari");
	smatch matches;

	if (regex_search(ua, matches, firefox)) {
		browser.first.assign("Firefox");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, msie)) {
		browser.first.assign("Internet Explorer");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, chrome)) {
		browser.first.assign("Chrome");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, opera)) {
		browser.first.assign("Opera");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, safari_mobile)) {
		browser.first.assign("Safari");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, safari)) {
		browser.first.assign("Safari");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	return false;
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
	if (conf.get(data_directory, "data-directory", "browser_stats", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string browser_stats_schema;
	if (conf.get(browser_stats_schema, "browser-stats-schema", "browser_stats")
			!= configuration::OK)
	{
		cerr << argv[0] << ": browser-stats-schema required" << endl;
		return 1;
	}

	string browser_version_stats_schema;
	if (conf.get(browser_version_stats_schema,
				 "browser-version-stats-schema",
				 "browser_stats")
			!= configuration::OK)
	{
		cerr << argv[0] << ": browser-version-stats-schema required" << endl;
		return 1;
	}

	string local_networks_string;
	if (conf.get(local_networks_string,
				 "local-networks",
				 "browser_stats",
				 true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid local-networks" << endl;
		return 1;
	}
	std::vector <std::pair <uint32_t, uint32_t> > local_networks;
	getLocalNetworks(local_networks, local_networks_string);


	boost::shared_ptr<StrftimeReadEnumerator> request_enum(
		new StrftimeReadEnumerator);

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

	// stats structures
	typedef map<uint32_t, request_stat_t> request_stats_t;
	request_stats_t request_stats;

	// collect the stats
	cerr << "Reading requests..." << endl;
	HTTPRequest http_request;
	ErrorStatus reader_status;
	size_t rows(0);
	while ((reader_status = request_reader.read(http_request)) == E_SUCCESS) {
		++rows;
		if (!isInternal(http_request.source_ip(), local_networks)) {
			continue;
		}
		request_stats[http_request.source_ip()].user_agents.insert(
			http_request.user_agent());
	}
	cerr << "Requests read: " << rows << endl;
	if (reader_status != E_EOF) {
		cerr << argv[0] << ": error reading requests!" << endl;
		return 1;
	}

	//map<string, size_t> user_agent_usage;
	map<string, size_t> browser_usage;
	map<string, map<string, size_t> > browser_version_usage;
	pair<string, string> browser;
	set<string> browsers;
	for (request_stats_t::iterator i(request_stats.begin());
		 i != request_stats.end();
		 ++i)
	{
		for (set<string>::const_iterator ua(i->second.user_agents.begin());
			 ua != i->second.user_agents.end();
			 ++ua)
		{
			//++user_agent_usage[*ua];
			if (!get_browser_from_user_agent(browser, *ua)) {
				continue;
			}

			i->second.browsers.insert(browser);
		}

		browsers.clear();
		for (set<pair<string, string> >::const_iterator b(
				i->second.browsers.begin());
			 b != i->second.browsers.end();
			 ++b)
		{
			browsers.insert(b->first);
			++(browser_version_usage[b->first][b->second]);
		}

		for (set<string>::const_iterator b(browsers.begin());
			 b != browsers.end();
			 ++b)
		{
			++(browser_usage[*b]);
		}
	}

#if 0
	regex search("(\\\\)|([\\b])|(\\f)|(\\n)|(\\r)|(\\t)|(\\v)");
	string replace("(?1\\\\\\\\)(?2\\\\b)(?3\\\\f)"
				   "(?4\\\\n)(?5\\\\r)(?6\\\\t)(?7\\\\v)");

	ostream_iterator<char> os_it(cout);

	for (map<string, size_t>::const_iterator i(user_agent_usage.begin());
		 i != user_agent_usage.end();
		 ++i)
	{
		/*
		regex_replace(os_it,
					  i->first.begin(),
					  i->first.end(),
					  search,
					  replace,
					  match_default | format_all);
		*/
		if (!get_browser_from_user_agent(browser, i->first)) {
			continue;
		}

		++(browser_version_usage[browser.browser][browser.version]);
	}
#endif

	PostgreSQLBulkWriter<browser_stat> writer(pg_conn,
											  browser_stats_schema,
											  date,
											  true);

	if (!writer) {
		cerr << argv[0] << ": writer initialization failed: " << endl;
		cerr << "\t" << writer.error() << endl;
		return 1;
	}

	browser_stat datum;
	for (map<string, size_t>::const_iterator i(browser_usage.begin());
		 i != browser_usage.end();
		 ++i)
	{
		datum.browser = i->first;
		datum.internal_host_count = i->second;
		writer.write(datum);
	}

	if (!writer.close()) {
		cerr << argv[0] << ": writer.close() failed: " << endl;
		cerr << "\t" << writer.error() << endl;
		return 1;
	}

	PostgreSQLBulkWriter<browser_version_stat> version_writer(pg_conn,
											  browser_version_stats_schema,
											  date,
											  true);

	if (!version_writer) {
		cerr << argv[0] << ": version_writer initialization failed: " << endl;
		cerr << "\t" << version_writer.error() << endl;
		return 1;
	}

	browser_version_stat version_datum;
	for (map<string, map<string, size_t> >::const_iterator i(
			browser_version_usage.begin());
		 i != browser_version_usage.end();
		 ++i)
	{
		for (map<string, size_t>::const_iterator j(i->second.begin());
			 j != i->second.end();
			 ++j)
		{
			version_datum.browser = i->first;
			version_datum.version = j->first;
			version_datum.internal_host_count = j->second;
			version_writer.write(version_datum);
		}
	}

	if (!version_writer.close()) {
		cerr << argv[0] << ": version_writer.close() failed: " << endl;
		cerr << "\t" << version_writer.error() << endl;
		return 1;
	}

	return 0;
}
