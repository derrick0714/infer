#include <iostream>
#include <vector>
#include <tr1/unordered_set>

#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include "NeoflowHTTPCorrelator.hpp"
#include "NeoflowDNSCorrelator.hpp"
#include "HTTPNeoflowCorrelator.hpp"
#include "HTTPDNSCorrelator.hpp"

#include "configuration.hpp"
#include "timeStamp.h"
#include "IPv4Network.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "IPv4FlowMatcher.hpp"
#include "NewIPv4FlowMatcher.hpp"
#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "DNS.hpp"
#include "FlowStats.hpp"
#include "PostgreSQLWriter.hpp"
#include "SetWriter.hpp"

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
			//throw validation_error("invalid timestamp");
			throw validation_error(validation_error::invalid_option_value);
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
		//throw validation_error("invalid port or port range");
		throw validation_error(validation_error::invalid_option_value);
	}

	if (ports.first > ports.second) {
		//throw validation_error("invalid port or port range");
		throw validation_error(validation_error::invalid_option_value);
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
				("/usr/local/etc/infer.conf"),
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
		("source-network", value<vector<IPv4Network> >(),
			"source IP address or CIDR block")
		("destination-network", value<vector<IPv4Network> >(),
			"destination IP address or CIDR block")
		("port", value<vector<std::pair<uint16_t, uint16_t> > >(),
			"port number or range")
		("source-port", value<vector<pair<uint16_t, uint16_t> > >(),
			"source port number or range")
		("destination-port", value<vector<pair<uint16_t, uint16_t> > >(),
			"destination port number or range")
		("protocol", value<vector<uint16_t> >(), "protocol number")
	;

	options_description desc_dns("DNS-based filters");
	desc_dns.add_options()
		("domain", value<vector<string> >(), "domain name")
		("source-domain", value<vector<string> >(), "source domain name")
		("destination-domain", value<vector<string> >(), "destination domain")
	;

	options_description desc_http("HTTP-based filters");
	desc_http.add_options()
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
		(vm.count("source-network") || vm.count("destination-network") ||
			vm.count("source-domain") || vm.count("destination-domain")))
	{
		cerr << "invalid filter combination" << endl
			 << "\tip cannot be used with source-network,"
			 	"destination-network, source-domain, or destination-domain"
			 << endl;
		return 1;
	}

	if (vm.count("domain") && 
		(vm.count("source-domain") || vm.count("destination-domain") ||
			vm.count("source-network") || vm.count("destination-network")))
	{
		cerr << "invalid filter combination" << endl
			 << "\tdomain cannot be used with source-domain,"
				"destination-domain, source-network, or destination-network"
			 << endl;
		return 1;
	}

	bool httpSearch(false);
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
	if (conf.get(data_directory, "data-directory", "search", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string result_schema;
	if (conf.get(result_schema, "result-schema", "search")
			!= configuration::OK)
	{
		cerr << argv[0] << ": result-schema required" << endl;
		return 1;
	}

	vector<string> dns_server_strings;
	status = conf.get(dns_server_strings, "dns-server", "search");
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

	PostgreSQLWriter <SearchNeoflowResult> statsWriter(pgConn,
													   result_schema,
													   vm["query-id"].as<std::string>() + "_neoflow");
	if (!statsWriter) {
		cerr << "Error: PostgreSQLWriter: " << statsWriter.error() << endl;
		return 1;
	}

	PostgreSQLWriter <SearchHTTPResult> httpWriter(pgConn,
												   result_schema,
												   vm["query-id"].as<std::string>() + "_http");
	if (!httpWriter) {
		cerr << "Error: PostgreSQLWriter: " << httpWriter.error() << endl;
		return 1;
	}

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

	cout << "DEBUG: data dir: " << data_directory << endl;
	StrftimeReadEnumerator dnsEnumerator(data_directory,
										 "%Y/%m/%d/dns_%H",
										 start_time,
										 end_time);
	if (!dnsEnumerator) {
		cerr << "Error: " << dnsEnumerator.error() << endl;
		return 1;
	}

	NewIPv4FlowMatcher new_matcher;
	bool domain_found(false), src_domain_found(false), dst_domain_found(false);
	if (vm.count("domain")) {
		cout << "DEBUG: going to get domains..." << endl;
		// add ips domain resolved to to flow new_matcher.
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
			if (!new_matcher.addEitherNetwork(tmpNet)) {
				abort();
			}
		}
		if (!ips.empty()) {
			domain_found = true;
		}
	}
	else if (vm.count("source-domain") || vm.count("destination-domain")) {
		// add ips accordingly
		cout << "DEBUG: going to get domains..." << endl;
		// add ips domain resolved to to flow new_matcher.
		vector<string> src_domains;
		if (vm.count("source-domain")) {
			src_domains = vm["source-domain"].as<vector<string> >();
		}
		vector<string> dst_domains;
		if (vm.count("destination-domain")) {
			dst_domains = vm["destination-domain"].as<vector<string> >();
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
			if (!new_matcher.addSourceNetwork(tmpNet)) {
				abort();
			}
		}
		if (!src_ips.empty()) {
			src_domain_found = true;
		}
		for (unordered_set<uint32_t>::const_iterator ip(dst_ips.begin());
			 ip != dst_ips.end();
			 ++ip)
		{
			cout << "DEBUG: Destination IP: " << ntohl(*ip) << endl;
			tmpNet.rawSet(*ip, numeric_limits<uint32_t>::max());
			if (!new_matcher.addDestinationNetwork(tmpNet)) {
				abort();
			}
		}
		if (!dst_ips.empty()) {
			dst_domain_found = true;
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
			if (!new_matcher.addEitherNetwork(*it)) {
				abort();
			}
		}
	}
	else if (vm.count("domain") && !domain_found) {
		return 0;
	}
		

	if (vm.count("source-network")) {
		const vector<IPv4Network> &nets(vm["source-network"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Source Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!new_matcher.addSourceNetwork(*it)) {
				abort();
			}
		}
	}
	else if (vm.count("source-domain") && !src_domain_found) {
		return 0;
	}

	if (vm.count("destination-network")) {
		const vector<IPv4Network> &nets(vm["destination-network"].as<vector<IPv4Network> >());

		for (vector<IPv4Network>::const_iterator it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			cout << "DEBUG: Destination Network: " << it->network() << " / "
				 << "0x" << hex << it->netmask() << dec << endl;
			if (!new_matcher.addDestinationNetwork(*it)) {
				abort();
			}
		}
	}
	else if (vm.count("destination-domain") && !dst_domain_found) {
		return 0;
	}

	if (vm.count("port")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["port"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Port range: " << it->first << '-' << it->second << endl;
			if (!new_matcher.addEitherPortRange(*it)) {
				abort();
			}
		}
	}

	if (vm.count("source-port")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["source-port"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Source Port range: " << it->first << '-' << it->second << endl;
			if (!new_matcher.addSourcePortRange(*it)) {
				abort();
			}
		}
	}

	if (vm.count("destination-port")) {
		const vector<pair<uint16_t, uint16_t> > &foo(vm["destination-port"].as<vector<pair<uint16_t, uint16_t> > >());

		for (vector<pair<uint16_t, uint16_t> >::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Destination Port range: " << it->first << '-' << it->second << endl;
			if (!new_matcher.addDestinationPortRange(*it)) {
				abort();
			}
		}
	}

	if (vm.count("protocol")) {
		const vector<uint16_t> &foo(vm["protocol"].as<vector<uint16_t> >());

		for (vector<uint16_t>::const_iterator it(foo.begin());
			 it != foo.end();
			 ++it)
		{
			cout << "DEBUG: Protocol number: " << *it << endl;
			if (!new_matcher.addProtocolRange(pair<uint8_t, uint8_t>(*it, *it))) {
				abort();
			}
		}
	}
	IPv4FlowMatcher matcher(new_matcher.old());

	cout << "Start time: " << start_time.seconds() << endl;
	cout << "End time:   " << end_time.seconds() << endl;


	if (!httpSearch) {
		// Neoflow search
		set<SearchNeoflowResult> statsSet;
		SetWriter<SearchNeoflowResult> statsSetWriter(statsSet);


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
			 PostgreSQLWriter<SearchHTTPResult>
			> neoflowHTTPCorrelator;

		NeoflowDNSCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<SearchDNSResult>,
			 PostgreSQLWriter<SearchDNSResponseResult>
			> neoflowDNSCorrelator;

		FlatFileReader<FlowStats> statsReader;
		FlowStats flowStats;
		ErrorStatus errorStatus;
		uint32_t index(0);
		SearchNeoflowResult searchNeoflowResult;
		for (StrftimeReadEnumerator::const_iterator file(statsEnumerator.begin());
			 file != statsEnumerator.end();
			 ++file)
		{
			if (statsReader.open(*file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *file << "'" << endl;
				return 1;
			}
			while ((errorStatus = statsReader.read(flowStats)) == E_SUCCESS) {
				if (matcher.isMatch(flowStats)) {
					searchNeoflowResult.index(index);
					searchNeoflowResult.protocol(flowStats.protocol());
					searchNeoflowResult.rawSourceIP(flowStats.rawSourceIP());
					searchNeoflowResult.rawDestinationIP(flowStats.rawDestinationIP());
					searchNeoflowResult.rawSourcePort(flowStats.rawSourcePort());
					searchNeoflowResult.rawDestinationPort(flowStats.rawDestinationPort());
					searchNeoflowResult.startTime(flowStats.startTime());
					searchNeoflowResult.endTime(flowStats.endTime());
					searchNeoflowResult.numBytes(flowStats.numBytes());
					searchNeoflowResult.numPackets(flowStats.numPackets());

					statsWriter.write(searchNeoflowResult);
					statsSetWriter.write(searchNeoflowResult);

					++index;
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

			if (statsSet.empty()) {
				continue;
			}

			// HTTP correlation
			boost::shared_ptr<StrftimeReadEnumerator> request_enum(
				new StrftimeReadEnumerator);

			request_enum->init(data_directory,
						   "%Y/%m/%d/http_request_%H",
						   statsSet.begin()->startTime(),
						   statsSet.begin()->startTime() + TimeStamp(3600, 1));
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
						   statsSet.begin()->startTime(),
						   statsSet.begin()->startTime() + TimeStamp(3600, 1));
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

			neoflowHTTPCorrelator.init(&request_reader, &response_reader, &httpWriter, &statsSet);
			if (neoflowHTTPCorrelator.run() != 0) {
				cerr << neoflowHTTPCorrelator.error() << endl;
				return 1;
			}

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


			statsSet.clear();
		}
	}
	else {
		// HTTP search
		set<SearchHTTPResult> httpSet;
		SetWriter<SearchHTTPResult> httpSetWriter(httpSet);


		StrftimeReadEnumerator request_enum(data_directory,
											   "%Y/%m/%d/http_request_%H",
											   start_time,
											   end_time);
		if (!request_enum) {
			cerr << "Error: " << request_enum.error() << endl;
			return 1;
		}

		HTTPNeoflowCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<FlowStats>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<SearchNeoflowResult>
			> httpNeoflowCorrelator;

		HTTPDNSCorrelator
			<EnumeratedFileReader
				<FlatFileReader
					<DNS>,
				 StrftimeReadEnumerator
				 >,
			 PostgreSQLWriter<SearchDNSResult>,
			 PostgreSQLWriter<SearchDNSResponseResult>
			> httpDNSCorrelator;

		FlatFileReader<HTTPRequest> request_reader;
		FlatFileReader<HTTPResponse> response_reader;
		HTTPRequest request;
		HTTPResponse response;
		ErrorStatus errorStatus;
		uint32_t index(0);
		SearchHTTPResult searchHTTPResult;
		for (StrftimeReadEnumerator::const_iterator request_file(request_enum.begin());
			 request_file != request_enum.end();
			 ++request_file)
		{
			if (request_reader.open(*request_file) != E_SUCCESS) {
				cerr << "Error: Unable to open '" << *request_file << "'" << endl;
				return 1;
			}
			while ((errorStatus = request_reader.read(request)) == E_SUCCESS) {
				if (new_matcher.isMatch(request)) {
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

					searchHTTPResult.clear();
					searchHTTPResult.neoflow_index(index);
					searchHTTPResult.request(request);

					httpWriter.write(searchHTTPResult);
					httpSetWriter.write(searchHTTPResult);

					++index;
				}
			}
			if (errorStatus != E_EOF) {
				cerr << "Error: unable to read from '" << *request_file << "'" << endl;
				return 1;
			}
			if (request_reader.close() != E_SUCCESS) {
				cerr << "Error: unable to close '" << *request_file << "'" << endl;
				return 1;
			}

/* // HTTP search is based on headers only in requests, so there's no need to look at responses.
			string response_file_string(request_file->string().substr(request_file->string().length() - 10));
			boost::filesystem::path response_file(response_file_string);
			if (exists(response_file) && !vm.count("host") && !vm.count("uri") && !vm.count("referer")) {
				cerr << "DEBUG: response_file exists!" << endl;

				if (response_reader.open(response_file) != E_SUCCESS) {
					cerr << "Error: Unable to open '" << response_file << "'" << endl;
					return 1;
				}
				while ((errorStatus = response_reader.read(response)) == E_SUCCESS) {
					if (new_matcher.isMatch(response)) {
						searchHTTPResult.clear();
						searchHTTPResult.neoflow_index(index);
						searchHTTPResult.response(response);

						httpWriter.write(searchHTTPResult);
						httpSetWriter.write(searchHTTPResult);

						++index;
					}
				}
				if (errorStatus != E_EOF) {
					cerr << "Error: unable to read from '" << response_file << "'" << endl;
					return 1;
				}
				if (response_reader.close() != E_SUCCESS) {
					cerr << "Error: unable to close '" << response_file << "'" << endl;
					return 1;
				}
			}
*/
			if (httpSet.empty()) {
				continue;
			}

			// Neoflow correlation
			boost::shared_ptr<StrftimeReadEnumerator> neoflowEnum(
				new StrftimeReadEnumerator);
			cout << "DEBUG: httpSet.begin()->time().seconds(): " << httpSet.begin()->time().seconds() << endl;
			cout << "DEBUG: httpSet.rbegin()->time().seconds(): " << httpSet.rbegin()->time().seconds() << endl;

			neoflowEnum->init(data_directory,
						   "%Y/%m/%d/neoflow_%H",
						   httpSet.begin()->time() - TimeStamp(3600, 0),
						   httpSet.begin()->time() + TimeStamp(0, 1));
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

			httpNeoflowCorrelator.init(&reader, &statsWriter, &httpSet);
			if (httpNeoflowCorrelator.run() != 0) {
				cerr << httpNeoflowCorrelator.error() << endl;
				return 1;
			}
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

			httpSet.clear();
		}
	}

	return 0;
}
