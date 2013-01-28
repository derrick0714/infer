#include <iostream>
#include <vector>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include "NeoflowHTTPCorrelator.hpp"
//#include "NeoflowDNSCorrelator.hpp"
#include "HTTPNeoflowCorrelator.hpp"
//#include "HTTPDNSCorrelator.hpp"
#include "ConnectionSearchHTTP.hpp"
#include "ConnectionSearchHTTPRequest.hpp"
#include "ConnectionSearchHTTPResponse.hpp"
#include "ConnectionSearchHTTPRequestReference.hpp"
#include "ConnectionSearchHTTPResponseReference.hpp"
#include "ConnectionSearchConnection.hpp"
#include "ConnectionSearchNeoflow.hpp"
#include "ConnectionSearchNeoflowReference.hpp"

#include "timeStamp.h"
#include "IPv4Network.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "NewIPv4FlowMatcher.hpp"
#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "DNS.hpp"
#include "Connection.hpp"
#include "FlowStats.hpp"
#include "PostgreSQLWriter.hpp"
#include "SetWriter.hpp"
#include "HTTP.hpp"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "configuration.hpp"

using namespace std;
using namespace tr1;
using namespace boost;
using namespace boost::program_options;
using namespace boost::asio::ip;

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
			throw validation_error("invalid timestamp");
		}
	}

	tmpTimet = timegm(&tmpTime);

	v = boost::any(TimeStamp(static_cast <uint32_t>(tmpTimet), 0));
}

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
		throw validation_error("invalid CIDR block: bad IP address");
	}

	uint16_t mask(32);
	if (pos != string::npos) {
		// get mask
		try {
			mask = boost::lexical_cast<uint16_t>(s.substr(pos+1));
		} catch (const boost::bad_lexical_cast &e) {
			throw validation_error("invalid CIDR block: bad netmask");
		}

		if (mask == 0 || mask > 32) {
			throw validation_error("invalid CIDR block: netmask out of range");
		}
	}

	IPv4Network net;
	if (!net.set(ip.to_ulong(), 0xffffffff << (32 - mask))) {
		throw validation_error("invalid CIDR block: network/netmask mismatch");
	}

	v = boost::any(net);
}

namespace std {

void validate(boost::any &v,
			  const std::vector<std::string> &values,
			  std::pair<uint16_t, uint16_t> *, int)
{
	using namespace boost::program_options;

	validators::check_first_occurrence(v);

	const std::string &s = validators::get_single_string(values);

	string::size_type pos(s.find('-'));
	
	std::pair<uint16_t, uint16_t> ports;

	try {
		if (pos != std::string::npos) {
			// we have a range
			if (pos != 0) {
				ports.first = boost::lexical_cast<uint16_t>(s.substr(0,pos));
			} else {
				ports.first = std::numeric_limits<uint16_t>::min();
			}

			if (pos != s.size() - 1) {
				ports.second = boost::lexical_cast<uint16_t>(s.substr(pos+1));
			} else {
				ports.second = std::numeric_limits<uint16_t>::max();
			}
		} else {
			// no range
			ports.first = ports.second = boost::lexical_cast<uint16_t>(s);
		}
	} catch (const boost::bad_lexical_cast) {
		throw validation_error("invalid port or port range");
	}

	if (ports.first > ports.second) {
		throw validation_error("invalid port or port range");
	}

	v = boost::any(ports);
}

}// namespace std;

int main(int argc, char **argv) {
	options_description desc_gen("General options");
	desc_gen.add_options()
		("help", "display help message")
		("config-file", 
			value<std::string>()->default_value
				("/usr/local/etc/infer_connection_search.conf"),
			"specify configuration file")
	;

	options_description desc_req("Required options");
	desc_req.add_options()
		("start-time", value<TimeStamp>(), "start time")
		("end-time", value<TimeStamp>(), "end time")
		("query-id", value<std::string>(), "query ID")
	;

	options_description desc_neo("Flow-based filters");
	desc_neo.add_options()
		("network", value<vector<IPv4Network> >(),
			"IP address or CIDR block")
		("network-a", value<vector<IPv4Network> >(),
			"IP address or CIDR block of side A of the connection")
		("network-b", value<vector<IPv4Network> >(),
			"IP address or CIDR block of side B of the connection")
		("port", value<vector<std::pair<uint16_t, uint16_t> > >(),
			"port number or range")
		("port-a", value<vector<pair<uint16_t, uint16_t> > >(),
			"port number or range of side A of the connection")
		("port-b", value<vector<pair<uint16_t, uint16_t> > >(),
			"port number or range of side B of the connection")
		("protocol", value<vector<uint16_t> >(), "protocol number")
	;

	options_description desc_dns("DNS-based filters");
	desc_dns.add_options()
		("domain", value<vector<string> >(), "domain name")
		("domain-a", value<vector<string> >(), "source domain name")
		("domain-b", value<vector<string> >(), "destination domain")
	;

	options_description desc_http("HTTP-based filters");
	desc_http.add_options()
		("client-network", value<vector<IPv4Network> >(),
			"IP address or CIDR block of the client")
		("server-network", value<vector<IPv4Network> >(),
			"IP address or CIDR block of the server")
		("client-domain", value<vector<string> >(),
			"client domain name")
		("server-domain", value<vector<string> >(),
			"server domain name")
		("host", value<string>(), "host name")
		("uri", value<string>(), "uri")
		("referer", value<string>(), "referer")
	;

	options_description desc;
	desc.add(desc_gen);
	desc.add(desc_req);
	desc.add(desc_neo);
	desc.add(desc_dns);
	desc.add(desc_http);

	if (argc == 1) {
		cerr << desc << endl;
		return 1;
	}

	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
	}
	catch (error e) {
		cerr << e.what() << endl;
		return 1;
	}
	notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 0;
	}

	if (!vm.count("start-time")) {
		cerr << "Error: start-time is required" << endl;
		return 1;
	}
	TimeStamp start_time(vm["start-time"].as<TimeStamp>());

	if (!vm.count("end-time")) {
		cerr << "Error: end-time is required" << endl;
		return 1;
	}
	TimeStamp end_time(vm["end-time"].as<TimeStamp>());

	if (!vm.count("query-id")) {
		cerr << "Error: query-id is required" << endl;
		return 1;
	}

	if (vm.count("network") && 
		(vm.count("network-a") || vm.count("network-b") ||
			vm.count("domain-a") || vm.count("domain-b")))
	{
		cerr << "invalid filter combination" << endl
			 << "\tip cannot be used with network-a,"
			 	"network-b, domain-a, or domain-b"
			 << endl;
		return 1;
	}

	if (vm.count("domain") && 
		(vm.count("domain-a") || vm.count("domain-b") ||
			vm.count("network-a") || vm.count("network-b")))
	{
		cerr << "invalid filter combination" << endl
			 << "\tdomain cannot be used with domain-a,"
				"domain-b, network-a, or network-b"
			 << endl;
		return 1;
	}

	if ((vm.count("client-network") || vm.count("server-network") ||
		 vm.count("client-domain") || vm.count("server-domain")) &&
		(vm.count("domain") || vm.count("domain-a") || vm.count("domain-b") ||
		 vm.count("network") || vm.count("network-a") || vm.count("network-b")))
	{
		cerr << "invalid filter combination" << endl
			 << "\tclient-network, server-network, client-domain, and "
			    "server-domain cannot be used with domain, domain-a, domain-b, "
				"network, network-a, or network-b"
			 << endl;
		return 1;
	}

	bool httpSearch(false);
	bool httpClientServer(false);
	bool hasFilter(false);
	boost::regex hostRegex;
	if (vm.count("host")) {
		try {
			hostRegex.assign(vm["host"].as<std::string>());
		} catch (const regex_error &e) {
			cerr << "invalid host regex" << endl;
			return 1;
		}
		httpSearch = true;
	}

	boost::regex uriRegex;
	if (vm.count("uri")) {
		try {
			uriRegex.assign(vm["uri"].as<std::string>());
		} catch (const regex_error &e) {
			cerr << "invalid uri regex" << endl;
			return 1;
		}
		httpSearch = true;
	}

	boost::regex refererRegex;
	if (vm.count("referer")) {
		try {
			refererRegex.assign(vm["referer"].as<std::string>());
		} catch (const regex_error &e) {
			cerr << "invalid referer regex" << endl;
			return 1;
		}
		httpSearch = true;
	}

	// Done parsing command line

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
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
	if (conf.get(data_directory, "data-directory", "connection_search", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string result_schema;
	if (conf.get(result_schema, "result-schema", "connection_search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": result-schema required" << endl;
		return 1;
	}

	vector<string> dns_server_strings;
	status = conf.get(dns_server_strings, "dns-server", "connection_search");
	if (status != configuration::OK && status != configuration::BAD_PATH) 
	{
		cerr << argv[0] << ": invalid dns-server" << endl;
		return 1;
	}

	unordered_set<uint32_t> dnsServers;
	address_v4 ip;
	for (vector<string>::const_iterator ip_str(dns_server_strings.begin());
		 ip_str != dns_server_strings.end();
		 ++ip_str)
	{
		try {
			ip = address_v4::from_string(*ip_str);
		}
		catch (const boost::system::system_error &) {
			cerr << argv[0] << ": invalid dns-server: " << *ip_str << endl;
			return 1;
		}
		dnsServers.insert(ip.to_ulong());
	}
/*
	PostgreSQLWriter <SearchNeoflowResult> statsWriter(pgConn,
													   result_schema,
													   vm["query-id"].as<std::string>() + "_neoflow");
	if (!statsWriter) {
		cerr << "Error: PostgreSQLWriter: " << statsWriter.error() << endl;
		return 1;
	}
*/
	// Connections
	PostgreSQLWriter <ConnectionSearchConnection> connectionWriter(pgConn,
								result_schema,
								vm["query-id"].as<std::string>() + "_t_connection");
	if (!connectionWriter) {
		cerr << "Error: PostgreSQLWriter: " << connectionWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchNeoflow> flowWriter(pgConn,
								result_schema,
								vm["query-id"].as<std::string>() + "_t_neoflow");
	if (!flowWriter) {
		cerr << "Error: PostgreSQLWriter: " << flowWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchNeoflowReference> flowRefWriter(pgConn,
								result_schema,
								vm["query-id"].as<std::string>() + "_xref_connection_neoflow");
	if (!flowRefWriter) {
		cerr << "Error: PostgreSQLWriter: " << flowRefWriter.error() << endl;
		return 1;
	}

	// HTTPs
	PostgreSQLWriter <ConnectionSearchHTTP> httpWriter(pgConn,
								result_schema,
								vm["query-id"].as<std::string>() + "_t_http");
	if (!httpWriter) {
		cerr << "Error: PostgreSQLWriter: " << httpWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchHTTPRequest> requestWriter(pgConn,
						result_schema,
						vm["query-id"].as<std::string>() + "_t_http_request");
	if (!requestWriter) {
		cerr << "Error: PostgreSQLWriter: " << requestWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchHTTPResponse> responseWriter(pgConn,
						result_schema,
						vm["query-id"].as<std::string>() + "_t_http_response");
	if (!responseWriter) {
		cerr << "Error: PostgreSQLWriter: " << responseWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchHTTPRequestReference> requestRefWriter(pgConn,
						result_schema,
						vm["query-id"].as<std::string>() + "_xref_http_http_request");
	if (!requestRefWriter) {
		cerr << "Error: PostgreSQLWriter: " << requestRefWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <ConnectionSearchHTTPResponseReference> responseRefWriter(pgConn,
						result_schema,
						vm["query-id"].as<std::string>() + "_xref_http_http_response");
	if (!responseRefWriter) {
		cerr << "Error: PostgreSQLWriter: " << responseRefWriter.error() << endl;
		return 1;
	}

	// Correlations
	PostgreSQLWriter <ConnectionSearchConnectionHTTPReference> connHTTPRefWriter(pgConn,
								result_schema,
								vm["query-id"].as<std::string>() + "_xref_connection_http");
	if (!connHTTPRefWriter) {
		cerr << "Error: PostgreSQLWriter: " << connHTTPRefWriter.error() << endl;
		return 1;
	}

/*
	PostgreSQLWriter <SearchDNSResult> dnsWriter(pgConn,
												 result_schema,
												 vm["query-id"].as<std::string>() + "_dns");
	if (!dnsWriter) {
		cerr << "Error: PostgreSQLWriter: " << dnsWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <SearchDNSResponseResult> dnsResponseWriter(pgConn,
												   result_schema,
												   vm["query-id"].as<std::string>() + "_dns_response");
	if (!dnsResponseWriter) {
		cerr << "Error: PostgreSQLWriter: " << dnsResponseWriter.error() << endl;
		return 1;
	}
*/
	cout << "DEBUG: data dir: " << data_directory << endl;
	StrftimeReadEnumerator dnsEnumerator(data_directory,
										 "%Y/%m/%d/dns_%H",
										 start_time,
										 end_time);
	if (!dnsEnumerator) {
		cerr << "Error: " << dnsEnumerator.error() << endl;
		return 1;
	}

	NewIPv4FlowMatcher matcher;
	if (vm.count("domain")) {
		cout << "DEBUG: going to get domains..." << endl;
		// add ips domain resolved to to flow matcher.
		const vector<string> &domains(
			vm["domain"].as<vector<string> >());
		unordered_set<string> names(domains.begin(), domains.end());
		unordered_set<uint32_t> ips;
		uint32_t tmpIP;
		FlatFileReader<DNS> dnsReader;
		DNS dns;
		ErrorStatus errorStatus;
		/*
		for (vector<string>::iterator dom(domains.begin());
			 dom != domains.end();
			 ++domains)
		*/
		for (StrftimeReadEnumerator::const_iterator file(dnsEnumerator.begin());
			 file != dnsEnumerator.end();
			 ++file)
		{
			if (dnsReader.open(*file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *file << "'" << endl;
				return 1;
			}
			cout << "DEBUG: opened " << file->string() << endl;
			while ((errorStatus = dnsReader.read(dns)) == E_SUCCESS) {
				for (std::vector<DNS::DNSResponse*>::const_iterator it(
						dns.responses().begin());
					 it != dns.responses().end();
					 ++it)
				{
					if (names.find((*it)->name()) == names.end()) {
						continue;
					}
					switch ((*it)->type()) {
					  case 1: // A_RECORD:
						tmpIP = *reinterpret_cast<const uint32_t *>((*it)->resourceData().data());
						if (ips.find(tmpIP) == ips.end()) {
							ips.insert(tmpIP);
						}
						break;
					  case 5: // CNAME_RECORD:
						if (names.find((*it)->resourceData()) == names.end()) {
							names.insert((*it)->resourceData());
						}
						break;
					  default:
						break;
					}
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << *file << "'" << endl;
				return 1;
			}
			if (dnsReader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << *file << "'" << endl;
				return 1;
			}
		}
		IPv4Network tmpNet;
		for (unordered_set<uint32_t>::const_iterator ip(ips.begin());
			 ip != ips.end();
			 ++ip)
		{
			cout << "DEBUG: IP: " << *ip << endl;
			tmpNet.rawSet(*ip, numeric_limits<uint32_t>::max());
			if (!matcher.addEitherNetwork(tmpNet)) {
				abort();
			}
			hasFilter = true;
		}

	}
	else if (vm.count("domain-a") || vm.count("domain-b") ||
			 vm.count("client-domain") || vm.count("server-domain")) {
		// add ips accordingly
		cout << "DEBUG: going to get domains..." << endl;
		// add ips domain resolved to to flow matcher.
		vector<string> src_domains;
		if (vm.count("domain-a")) {
			src_domains = vm["domain-a"].as<vector<string> >();
		}
		if (vm.count("client-domain")) {
			src_domains = vm["client-domain"].as<vector<string> >();
			httpSearch = true;
			httpClientServer = true;
		}
		vector<string> dst_domains;
		if (vm.count("domain-b")) {
			dst_domains = vm["domain-b"].as<vector<string> >();
		}
		if (vm.count("server-domain")) {
			dst_domains = vm["server-domain"].as<vector<string> >();
			httpSearch = true;
			httpClientServer = true;
		}
		unordered_set<string> src_names(src_domains.begin(), src_domains.end()),
							  dst_names(dst_domains.begin(), dst_domains.end());
		unordered_set<uint32_t> src_ips, dst_ips;
		uint32_t tmpIP;
		FlatFileReader<DNS> dnsReader;
		DNS dns;
		ErrorStatus errorStatus;
		/*
		for (vector<string>::iterator dom(domains.begin());
			 dom != domains.end();
			 ++domains)
		*/
		for (StrftimeReadEnumerator::const_iterator file(dnsEnumerator.begin());
			 file != dnsEnumerator.end();
			 ++file)
		{
			if (dnsReader.open(*file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *file << "'" << endl;
				return 1;
			}
			cout << "DEBUG: opened " << file->string() << endl;
			while ((errorStatus = dnsReader.read(dns)) == E_SUCCESS) {
				for (std::vector<DNS::DNSResponse*>::const_iterator it(
						dns.responses().begin());
					 it != dns.responses().end();
					 ++it)
				{
					if (src_names.find((*it)->name()) != src_names.end()) {
						switch ((*it)->type()) {
						  case 1: // A_RECORD:
							tmpIP = *reinterpret_cast<const uint32_t *>((*it)->resourceData().data());
							if (src_ips.find(tmpIP) == src_ips.end()) {
								src_ips.insert(tmpIP);
							}
							break;
						  case 5: // CNAME_RECORD:
							if (src_names.find((*it)->resourceData()) == src_names.end()) {
								src_names.insert((*it)->resourceData());
							}
							break;
						  default:
							break;
						}
					}
					if (dst_names.find((*it)->name()) != dst_names.end()) {
						switch ((*it)->type()) {
						  case 1: // A_RECORD:
							tmpIP = *reinterpret_cast<const uint32_t *>((*it)->resourceData().data());
							if (dst_ips.find(tmpIP) == dst_ips.end()) {
								dst_ips.insert(tmpIP);
							}
							break;
						  case 5: // CNAME_RECORD:
							if (dst_names.find((*it)->resourceData()) == dst_names.end()) {
								dst_names.insert((*it)->resourceData());
							}
							break;
						  default:
							break;
						}
					}
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << *file << "'" << endl;
				return 1;
			}
			if (dnsReader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << *file << "'" << endl;
				return 1;
			}
		}
		IPv4Network tmpNet;
		for (unordered_set<uint32_t>::const_iterator ip(src_ips.begin());
			 ip != src_ips.end();
			 ++ip)
		{
			cout << "DEBUG: Source IP: " << ntohl(*ip) << endl;
			tmpNet.rawSet(*ip, numeric_limits<uint32_t>::max());
			if (!matcher.addSourceNetwork(tmpNet)) {
				abort();
			}
			hasFilter = true;
		}
		for (unordered_set<uint32_t>::const_iterator ip(dst_ips.begin());
			 ip != dst_ips.end();
			 ++ip)
		{
			cout << "DEBUG: Destination IP: " << ntohl(*ip) << endl;
			tmpNet.rawSet(*ip, numeric_limits<uint32_t>::max());
			if (!matcher.addDestinationNetwork(tmpNet)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("network")) {
		const vector<IPv4Network> &nets(vm["network"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!matcher.addEitherNetwork(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("network-a")) {
		const vector<IPv4Network> &nets(vm["network-a"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Source Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!matcher.addSourceNetwork(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("network-b")) {
		const vector<IPv4Network> &nets(vm["network-b"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Destination Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!matcher.addDestinationNetwork(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("client-network")) {
		const vector<IPv4Network> &nets(vm["client-network"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Source Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!matcher.addSourceNetwork(*it)) {
				abort();
			}
			hasFilter = true;
		}
		httpSearch = true;
		httpClientServer = true;
	}

	if (vm.count("server-network")) {
		const vector<IPv4Network> &nets(vm["server-network"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Destination Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!matcher.addDestinationNetwork(*it)) {
				abort();
			}
			hasFilter = true;
		}
		httpSearch = true;
		httpClientServer = true;
	}

	if (vm.count("port")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["port"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Port range: " << it->first << '-' << it->second << endl;
			if (!matcher.addEitherPortRange(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("port-a")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["port-a"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Source Port range: " << it->first << '-' << it->second << endl;
			if (!matcher.addSourcePortRange(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("port-b")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["port-b"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Destination Port range: " << it->first << '-' << it->second << endl;
			if (!matcher.addDestinationPortRange(*it)) {
				abort();
			}
			hasFilter = true;
		}
	}

	if (vm.count("protocol")) {
		const vector<uint16_t> &foo(vm["protocol"].as<vector<uint16_t> >());

		for (vector<uint16_t>::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Protocol number: " << *it << endl;
			if (!matcher.addProtocolRange(pair<uint8_t, uint8_t>(*it, *it))) {
				abort();
			}
			hasFilter = true;
		}
	}

	NewIPv4FlowMatcher reverseMatcher(matcher.reverse());

	cout << "Start time: " << start_time.seconds() << endl;
	cout << "End time:   " << end_time.seconds() << endl;


	if (!httpSearch) {
		IPv4FlowMatcher oldMatcher(matcher.old());
		IPv4FlowMatcher oldReverseMatcher(reverseMatcher.old());

		// Neoflow search
		unordered_map<string, Connection> connMap;
		set<ConnectionSearchConnection> connSet;
		string flowID;
		uint8_t id_protocol;
		uint32_t id_src_ip;
		uint32_t id_dst_ip;
		uint16_t id_src_port;
		uint16_t id_dst_port;
		uint32_t conn_id(0), neoflow_id(0);

		StrftimeReadEnumerator statsEnumerator(data_directory,
											   "%Y/%m/%d/neoflow_%H",
											   start_time,
											   end_time);
		if (!statsEnumerator) {
			cerr << "Error: " << statsEnumerator.error() << endl;
			return 1;
		}

		NeoflowHTTPCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<HTTPRequest>,
				 StrftimeReadEnumerator
				 >,
			 EnumeratedFileReader
				<FlatFileReader
					<HTTPResponse>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<ConnectionSearchHTTP>,
			 PostgreSQLWriter<ConnectionSearchHTTPRequest>,
			 PostgreSQLWriter<ConnectionSearchHTTPResponse>,
			 PostgreSQLWriter<ConnectionSearchHTTPRequestReference>,
			 PostgreSQLWriter<ConnectionSearchHTTPResponseReference>,
			 PostgreSQLWriter<ConnectionSearchConnectionHTTPReference>
			> neoflowHTTPCorrelator;

/*
		NeoflowDNSCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<SearchDNSResult>,
			 PostgreSQLWriter<SearchDNSResponseResult>
			> neoflowDNSCorrelator;
*/

		FlatFileReader<FlowStats> statsReader;
		FlowStats flowStats;
		ErrorStatus errorStatus;
		ConnectionSearchConnection connectionSearchConnection;
		ConnectionSearchNeoflow connectionSearchNeoflow;
		ConnectionSearchNeoflowReference connFlowRef;
		for (StrftimeReadEnumerator::const_iterator file(statsEnumerator.begin());
			 file != statsEnumerator.end();
			 ++file)
		{
			if (statsReader.open(*file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *file << "'" << endl;
				return 1;
			}
			while ((errorStatus = statsReader.read(flowStats)) == E_SUCCESS) {
				if (oldMatcher.isMatch(flowStats) || oldReverseMatcher.isMatch(flowStats)) {
					// formulate the map key
					id_protocol = flowStats.protocol();
					flowID.assign(reinterpret_cast<char *>(&id_protocol),
									  sizeof(id_protocol));
					if (flowStats.rawSourceIP() < flowStats.rawDestinationIP())
					{
						id_src_ip = flowStats.rawSourceIP();
						flowID.append(reinterpret_cast<char *>(&id_src_ip),
										  sizeof(id_src_ip));
						id_dst_ip = flowStats.rawDestinationIP();
						flowID.append(reinterpret_cast<char *>(&id_dst_ip),
										  sizeof(id_dst_ip));
						id_src_port = flowStats.rawSourcePort();
						flowID.append(reinterpret_cast<char *>(&id_src_port),
										  sizeof(id_src_port));
						id_dst_port = flowStats.rawDestinationPort();
						flowID.append(reinterpret_cast<char *>(&id_dst_port),
										  sizeof(id_dst_port));
					}
					else {
						id_src_ip = flowStats.rawDestinationIP();
						flowID.append(reinterpret_cast<char *>(&id_src_ip),
										  sizeof(id_src_ip));
						id_dst_ip = flowStats.rawSourceIP();
						flowID.append(reinterpret_cast<char *>(&id_dst_ip),
										  sizeof(id_dst_ip));
						id_src_port = flowStats.rawDestinationPort();
						flowID.append(reinterpret_cast<char *>(&id_src_port),
										  sizeof(id_src_port));
						id_dst_port = flowStats.rawSourcePort();
						flowID.append(reinterpret_cast<char *>(&id_dst_port),
										  sizeof(id_dst_port));
					}

					// check if there's an HTTP already in the map
					//      if not, create one
					unordered_map<string, Connection>::iterator connIt(connMap.find(flowID));
					if (connIt == connMap.end()) {
						connIt = connMap.insert(make_pair(flowID,
													Connection(id_protocol,
															   id_src_ip,
															   id_dst_ip,
															   id_src_port,
															   id_dst_port))).first;
					}

					// add this request to the Connection in the map
					connIt->second.add_flow(flowStats);
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << *file << "'" << endl;
				return 1;
			}
			if (statsReader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << *file << "'" << endl;
				return 1;
			}

			if (connMap.empty()) {
				continue;
			}

			// Write the Connection results
			for (unordered_map<string, Connection>::iterator it(connMap.begin());
				 it != connMap.end();
				 ++it)
			{
				connectionSearchConnection.connection_id(conn_id);
				connectionSearchConnection.start_time(it->second.start_time());
				connectionSearchConnection.end_time(it->second.end_time());
				connectionSearchConnection.protocol(it->second.protocol());
				connectionSearchConnection.ip_a(it->second.ip_a());
				connectionSearchConnection.ip_b(it->second.ip_b());
				connectionSearchConnection.port_a(it->second.port_a());
				connectionSearchConnection.port_b(it->second.port_b());

				connectionWriter.write(connectionSearchConnection);
				connSet.insert(connectionSearchConnection);

				vector<FlowStats> flows(it->second.flows());
				connFlowRef.connection_id(conn_id);
				for (vector<FlowStats>::const_iterator flow(flows.begin());
					 flow != flows.end();
					 ++flow)
				{
					connectionSearchNeoflow.neoflow_id(neoflow_id);
					connectionSearchNeoflow.start_time(flow->startTime());
					connectionSearchNeoflow.end_time(flow->endTime());
					connectionSearchNeoflow.protocol(flow->protocol());
					connectionSearchNeoflow.source_ip(flow->sourceIP());
					connectionSearchNeoflow.destination_ip(flow->destinationIP());
					connectionSearchNeoflow.source_port(flow->sourcePort());
					connectionSearchNeoflow.destination_port(flow->destinationPort());
					connectionSearchNeoflow.packet_count(flow->numPackets());
					connectionSearchNeoflow.byte_count(flow->numBytes());

					flowWriter.write(connectionSearchNeoflow);

					connFlowRef.neoflow_id(neoflow_id);

					flowRefWriter.write(connFlowRef);

					++neoflow_id;
				}

				++conn_id;
			}

			// HTTP correlation
			boost::shared_ptr<StrftimeReadEnumerator> request_enum(
				new StrftimeReadEnumerator);

			request_enum->init(data_directory,
						   "%Y/%m/%d/http_request_%H",
						   connSet.begin()->start_time(),
						   connSet.begin()->start_time() + TimeStamp(3600, 1));
			if (!(*request_enum)) {
				cerr << "Error: Unable to initialize read enumerator" << endl;
				return 1;
			}

			EnumeratedFileReader
				<FlatFileReader
					<HTTPRequest>,
				 StrftimeReadEnumerator
				> request_reader;
			errorStatus = request_reader.init(request_enum);
			if (errorStatus != E_SUCCESS && errorStatus != E_EOF) {
				cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
				return 1;
			}

			boost::shared_ptr<StrftimeReadEnumerator> response_enum(
				new StrftimeReadEnumerator);

			response_enum->init(data_directory,
						   "%Y/%m/%d/http_response_%H",
						   connSet.begin()->start_time(),
						   connSet.begin()->start_time() + TimeStamp(3600, 1));
			if (!(*response_enum)) {
				cerr << "Error: Unable to initialize read enumerator" << endl;
				return 1;
			}

			EnumeratedFileReader
				<FlatFileReader
					<HTTPResponse>,
				 StrftimeReadEnumerator
				> response_reader;
			errorStatus = response_reader.init(response_enum);
			if (errorStatus != E_SUCCESS && errorStatus != E_EOF) {
				cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
				return 1;
			}

			neoflowHTTPCorrelator.init(&request_reader,
									   &response_reader,
									   &httpWriter,
									   &requestWriter,
									   &responseWriter,
									   &requestRefWriter,
									   &responseRefWriter,
									   &connHTTPRefWriter,
									   &connSet);
			if (neoflowHTTPCorrelator.run() != 0) {
				cerr << neoflowHTTPCorrelator.error() << endl;
				return 1;
			}

			/* TODO 
			// DNS correlation
			boost::shared_ptr<StrftimeReadEnumerator> dnsEnum(
				new StrftimeReadEnumerator);

			dnsEnum->init(data_directory,
						  "%Y/%m/%d/dns_%H",
						  statsSet.begin()->startTime() - TimeStamp(3600, 0),
						  statsSet.begin()->startTime() + TimeStamp(0, 1));
			if (!(*dnsEnum)) {
				cerr << "Error: Unable to initialize read enumerator" << endl;
				return 1;
			}

			EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				> dnsReader;
			errorStatus = dnsReader.init(dnsEnum);
			if (errorStatus != E_SUCCESS && errorStatus != E_EOF) {
				cerr << "Error: Unable to initialize EnumeratedFileReader (DNS)" << endl;
				return 1;
			}

			neoflowDNSCorrelator.init(&dnsReader, &dnsWriter, &dnsResponseWriter, &statsSet, &dnsServers);
			if (neoflowDNSCorrelator.run() != 0) {
				cerr << neoflowDNSCorrelator.error() << endl;
				return 1;
			}

			*/

			connMap.clear();
			connSet.clear();
		}
	}
	else {
		// HTTP search
		StrftimeReadEnumerator requestEnumerator(data_directory,
											   "%Y/%m/%d/http_request_%H",
											   start_time,
											   end_time);
		if (!requestEnumerator) {
			cerr << "Error: " << requestEnumerator.error() << endl;
			return 1;
		}
		HTTPNeoflowCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<FlowStats>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<ConnectionSearchConnection>,
			 PostgreSQLWriter<ConnectionSearchNeoflow>,
			 PostgreSQLWriter<ConnectionSearchNeoflowReference>,
			 PostgreSQLWriter<ConnectionSearchConnectionHTTPReference>
			> httpNeoflowCorrelator;

/*
		HTTPDNSCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<SearchDNSResult>,
			 PostgreSQLWriter<SearchDNSResponseResult>
			> httpDNSCorrelator;
*/
		FlatFileReader<HTTPRequest> requestReader;
		FlatFileReader<HTTPResponse> responseReader;
		string httpFlowID;
		uint8_t id_protocol;
		uint32_t id_src_ip;
		uint32_t id_dst_ip;
		uint16_t id_src_port;
		uint16_t id_dst_port;
		HTTPRequest request;
		HTTPResponse response;
		ErrorStatus errorStatus;
		uint32_t http_id(0), http_request_id(0), http_response_id(0);
		unordered_map<string, HTTP> httpMap;
		set<ConnectionSearchHTTP> httpSet;
		ConnectionSearchHTTP connectionSearchHTTP;
		ConnectionSearchHTTPRequest requestRow;
		ConnectionSearchHTTPResponse responseRow;
		ConnectionSearchHTTPRequestReference requestRef;
		ConnectionSearchHTTPResponseReference responseRef;
		for (StrftimeReadEnumerator::const_iterator file(requestEnumerator.begin());
			 file != requestEnumerator.end();
			 ++file)
		{
			// go through all the requests
			if (requestReader.open(*file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *file << "'" << endl;
				return 1;
			}
			while ((errorStatus = requestReader.read(request)) == E_SUCCESS) {
				if (matcher.isMatch(request) || (!httpClientServer && reverseMatcher.isMatch(request))) {
					if (vm.count("host")) {
						if (!boost::regex_match(request.host(), hostRegex)) {
							continue;
						}
					}
					if (vm.count("uri")) {
						if (!boost::regex_match(request.uri(), uriRegex)) {
							continue;
						}
					}
					if (vm.count("referer")) {
						if (!boost::regex_match(request.referer(), refererRegex)) {
							continue;
						}
					}

					// we have a match. let's keep track of it
					
					// formulate the map key
					id_protocol = request.protocol();
					httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
									  sizeof(id_protocol));
					id_src_ip = request.raw_source_ip();
					httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
									  sizeof(id_src_ip));
					id_dst_ip = request.raw_destination_ip();
					httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
									  sizeof(id_dst_ip));
					id_src_port = request.raw_source_port();
					httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
									  sizeof(id_src_port));
					id_dst_port = request.raw_destination_port();
					httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
									  sizeof(id_dst_port));

					// check if there's an HTTP already in the map
					//      if not, create one
					unordered_map<string, HTTP>::iterator httpIt(httpMap.find(httpFlowID));
					if (httpIt == httpMap.end()) {
						httpIt = httpMap.insert(make_pair(httpFlowID,
														  HTTP(id_protocol,
															   id_src_ip,
															   id_dst_ip,
															   id_src_port,
															   id_dst_port))).first;
					}

					// add this request to the HTTP in the map
					httpIt->second.add_request(request);
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << *file << "'" << endl;
				return 1;
			}
			if (requestReader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << *file << "'" << endl;
				return 1;
			}
			
			// now go through all the responses
			string hour(file->string().substr(file->string().length() - 3));
			string responseFile(file->string().substr(0, file->string().length() - 10));
			responseFile.append("response" + hour);
			cout << "DEBUG: " << responseFile << endl;
			if (responseReader.open(responseFile) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << responseFile << "'" << endl;
				return 1;
			}
			while ((errorStatus = responseReader.read(response)) == E_SUCCESS) {
				// formulate the map key
				id_protocol = response.protocol();
				httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
								  sizeof(id_protocol));
				id_dst_ip = response.raw_destination_ip();
				httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
								  sizeof(id_dst_ip));
				id_src_ip = response.raw_source_ip();
				httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
								  sizeof(id_src_ip));
				id_dst_port = response.raw_destination_port();
				httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
								  sizeof(id_dst_port));
				id_src_port = response.raw_source_port();
				httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
								  sizeof(id_src_port));

				unordered_map<string, HTTP>::iterator httpIt(httpMap.find(httpFlowID));
				if (httpIt != httpMap.end()) {
					// add it
					httpIt->second.add_response(response);
				}
				else if (hasFilter && (reverseMatcher.isMatch(response) || (!httpClientServer && matcher.isMatch(response)))) {
					// we have a match. let's keep track of it
					
					// check if there's an HTTP already in the map
					//      if not, create one
					httpIt = httpMap.insert(make_pair(httpFlowID,
													  HTTP(id_protocol,
														   id_dst_ip,
														   id_src_ip,
														   id_dst_port,
														   id_src_port))).first;

					// add this response to the HTTP in the map
					httpIt->second.add_response(response);
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << responseFile << "'" << endl;
				return 1;
			}
			if (responseReader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << responseFile << "'" << endl;
				return 1;
			}
			
			// at this point, the httpMap is populated with matching HTTPs
			if (httpMap.empty()) {
				continue;
			}

			// write the HTTP results
			for (unordered_map<string, HTTP>::iterator it(httpMap.begin());
				 it != httpMap.end();
				 it = httpMap.erase(it))
			{
				connectionSearchHTTP.http_id(http_id);
				connectionSearchHTTP.start_time(it->second.start_time());
				connectionSearchHTTP.end_time(it->second.end_time());
				connectionSearchHTTP.protocol(it->second.protocol());
				connectionSearchHTTP.client_ip(it->second.client_ip());
				connectionSearchHTTP.server_ip(it->second.server_ip());
				connectionSearchHTTP.client_port(it->second.client_port());
				connectionSearchHTTP.server_port(it->second.server_port());

				httpWriter.write(connectionSearchHTTP);
				httpSet.insert(connectionSearchHTTP);

				// write the requests
				vector<HTTPRequest> requests(it->second.requests());
				requestRef.http_id(http_id);
				for (vector<HTTPRequest>::const_iterator req(requests.begin());
					 req != requests.end();
					 ++req)
				{
					requestRow.http_request_id(http_request_id);
					requestRow.time(req->time());
					requestRow.protocol(req->protocol());
					requestRow.source_ip(req->source_ip());
					requestRow.destination_ip(req->destination_ip());
					requestRow.source_port(req->source_port());
					requestRow.destination_port(req->destination_port());
					requestRow.type(req->type());
					requestRow.uri(req->uri());
					requestRow.version(req->version());
					requestRow.host(req->host());
					requestRow.user_agent(req->user_agent());
					requestRow.referer(req->referer());

					requestWriter.write(requestRow);

					requestRef.http_request_id(http_request_id);

					requestRefWriter.write(requestRef);

					++http_request_id;
				}

				// write the responses
				vector<HTTPResponse> responses(it->second.responses());
				responseRef.http_id(http_id);
				for (vector<HTTPResponse>::const_iterator res(responses.begin());
					 res != responses.end();
					 ++res)
				{
					responseRow.http_response_id(http_response_id);
					responseRow.time(res->time());
					responseRow.protocol(res->protocol());
					responseRow.source_ip(res->source_ip());
					responseRow.destination_ip(res->destination_ip());
					responseRow.source_port(res->source_port());
					responseRow.destination_port(res->destination_port());
					responseRow.version(res->version());
					responseRow.status(res->status());
					responseRow.response(res->response());
					responseRow.reason(res->reason());
					responseRow.content_type(res->content_type());

					responseWriter.write(responseRow);

					responseRef.http_response_id(http_response_id);

					responseRefWriter.write(responseRef);

					++http_response_id;
				}

				++http_id;
			}

			// Neoflow correlation
			boost::shared_ptr<StrftimeReadEnumerator> neoflowEnum(
				new StrftimeReadEnumerator);
			cout << "DEBUG: httpSet.begin()->start_time().seconds(): " << httpSet.begin()->start_time().seconds() << endl;
			cout << "DEBUG: httpSet.rbegin()->start_time().seconds(): " << httpSet.rbegin()->start_time().seconds() << endl;

			neoflowEnum->init(data_directory,
						   "%Y/%m/%d/neoflow_%H",
						   httpSet.begin()->start_time() - TimeStamp(3600, 0),
						   httpSet.begin()->end_time() + TimeStamp(0, 1));
			if (!(*neoflowEnum)) {
				cerr << "Error: Unable to initialize read enumerator" << endl;
				return 1;
			}

			EnumeratedFileReader
				<FlatFileReader
					<FlowStats>,
				 StrftimeReadEnumerator
				> reader;
			errorStatus = reader.init(neoflowEnum);
			if (errorStatus != E_SUCCESS && errorStatus != E_EOF) {
				cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
				return 1;
			}

			httpNeoflowCorrelator.init(&reader,
									   &connectionWriter,
									   &flowWriter,
									   &flowRefWriter,
									   &connHTTPRefWriter,
									   &httpSet);
			if (httpNeoflowCorrelator.run() != 0) {
				cerr << "HTTPNeoflowCorrelator error: " << endl;
				cerr << httpNeoflowCorrelator.error() << endl;
				return 1;
			}

			/* TODO
			// DNS correlation
			boost::shared_ptr<StrftimeReadEnumerator> dnsEnum(
				new StrftimeReadEnumerator);

			dnsEnum->init(data_directory,
						  "%Y/%m/%d/dns_%H",
						  httpSet.begin()->time() - TimeStamp(3600, 0),
						  httpSet.begin()->time() + TimeStamp(0, 1));
			if (!(*dnsEnum)) {
				cerr << "Error: Unable to initialize read enumerator" << endl;
				return 1;
			}

			EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				> dnsReader;
			errorStatus = dnsReader.init(dnsEnum);
			if (errorStatus != E_SUCCESS && errorStatus != E_EOF) {
				cerr << "Error: Unable to initialize EnumeratedFileReader (DNS)" << endl;
				return 1;
			}

			httpDNSCorrelator.init(&dnsReader, &dnsWriter, &dnsResponseWriter, &httpSet, &dnsServers);
			if (httpDNSCorrelator.run() != 0) {
				cerr << httpDNSCorrelator.error() << endl;
				return 1;
			}
			*/

			httpMap.clear();
			httpSet.clear();
		}
	}

	return 0;
}
