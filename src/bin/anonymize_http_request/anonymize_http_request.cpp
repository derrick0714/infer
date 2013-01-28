#include <iostream>

#include <time.h>
#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include "fixed_time_functor.hpp"
#include "configuration.hpp"
#include "FlatFileReader.hpp"
#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "StrftimeWriteEnumerator.hpp"
#include "IPv4Network.hpp"
#include "HTTPRequest.h"
#include "rot13_5.hpp"

using namespace std;
using namespace boost::program_options;

std::istream & operator>> (std::istream& s, IPv4Network &net) {
	using namespace boost::asio::ip;

	std::string str;
	s >> str;

	string::size_type pos(str.find('/'));

	address_v4 ip;
	try {
		ip = address_v4::from_string(str.substr(0,pos));
	} catch (const boost::system::system_error &e) {
		//throw validation_error("invalid CIDR block: bad IP address");
		s.setstate(ios_base::failbit);
		return s;
	}

	uint16_t mask(32);
	if (pos != string::npos) {
		// get mask
		try {
			mask = boost::lexical_cast<uint16_t>(str.substr(pos+1));
		} catch (const boost::bad_lexical_cast &e) {
			//throw validation_error("invalid CIDR block: bad netmask");
			s.setstate(ios_base::failbit);
			return s;
		}

		if (mask == 0 || mask > 32) {
			//throw validation_error("invalid CIDR block: netmask out of range");
			s.setstate(ios_base::failbit);
			return s;
		}
	}

	if (!net.set(ip.to_ulong(), 0xffffffff << (32 - mask))) {
		//throw validation_error("invalid CIDR block: network/netmask mismatch");
		s.setstate(ios_base::failbit);
		return s;
	}

	return s;
}

int main(int argc, char **argv) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file", 
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("date", value<string>(), "the date (YYYY-mm-dd)")
		("hour", value<string>(), "the hour (HH)")
	;

	positional_options_description p;
	p.add("date", 1).add("hour", 1);
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

	if (!vm.count("hour")) {
		cerr << "Error: hour is required." << endl;
		cerr << desc_gen << endl;
		return 1;
	}
	date += " " + vm["hour"].as<string>();



	struct tm _tm;
	memset(&_tm, 0, sizeof(_tm));
	if (strptime(date.c_str(), "%Y-%m-%d %H", &_tm) == NULL) {
		cerr << "Error: invalid date: " << vm["date"].as<string>() << endl;
		return 1;
	}
	time_t tmp_time(timegm(&_tm));
	TimeStamp start_time(tmp_time, 0);

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string data_directory;
	if (conf.get(data_directory, "data-directory", "anonymize_http_request", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid data_directory" << endl;
		return 1;
	}
	
	string output_directory;
	if (conf.get(output_directory, "output-directory", "anonymize_http_request")
			!= configuration::OK)
	{
		cerr << argv[0] << ": output-directory required" << endl;
		return 1;
	}

	IPv4Network ip_xor;
	if (conf.get(ip_xor, "ip-xor", "anonymize_http_request") != configuration::OK) {
		cerr << argv[0] << ": ip-xor required" << endl;
		return 1;
	}

	vector<IPv4Network> internal_networks;
	if (conf.get(internal_networks,
				 "internal-network",
				 "anonymize_http_request") != configuration::OK)
	{
		cerr << argv[0] << ": invalid internal-network" << endl;
		return 1;
	}

	cerr << "conf: data-directory: " << data_directory << endl;
	cerr << "      date:           " << date << endl;
	cerr << "      start_time:     " << start_time.seconds() << '.' << start_time.microseconds() << endl;
	cerr << "      tmp_time:       " << tmp_time << endl;

	StrftimeReadEnumerator test_enum(output_directory,
									 "%Y/%m/%d/http_request_%H",
									 start_time,
									 start_time + TimeStamp(1, 0));
	if (test_enum.begin() != test_enum.end()) {
		cerr << "output file '" << test_enum.begin()->string()
			 << "' already exists!" << endl;
		return 1;
	}

	StrftimeReadEnumerator read_enum(data_directory,
									 "%Y/%m/%d/http_request_%H",
									 start_time,
									 start_time + TimeStamp(1, 0));
	if (read_enum.begin() == read_enum.end()) {
		cerr << "No files matching " << date << endl;
		return 1;
	}
	string infile_name(read_enum.begin()->string());

	fixed_time_functor start_time_functor(start_time);
	boost::shared_ptr<StrftimeWriteEnumerator<HTTPRequest, fixed_time_functor> >
		output_enumerator(new StrftimeWriteEnumerator<HTTPRequest,
													  fixed_time_functor>(
			output_directory, "%Y/%m/%d/http_request_%H", start_time_functor));

	InferFileWriter<FlatFileWriter<HTTPRequest>,
					StrftimeWriteEnumerator<HTTPRequest,
											fixed_time_functor> >
		output_writer(output_enumerator);

	FlatFileReader<HTTPRequest> reader;
	HTTPRequest request;
	ErrorStatus error_status;

	if (reader.open(infile_name) != E_SUCCESS) {
		cerr << argv[0] << ": unable to open '" << infile_name << "'" << endl;
		return 1;
	}

	bool src_xor(false), dst_xor(false);
	rot13_5 encrypt;
	while ((error_status = reader.read(request)) == E_SUCCESS) {
		// xor the internal IPs
		src_xor = dst_xor = false;
		for (vector<IPv4Network>::const_iterator net(internal_networks.begin());
			 net != internal_networks.end();
			 ++net)
		{
			if (!src_xor && 
					net->rawIsInNetwork(request.raw_source_ip()))
			{
				request.raw_source_ip(request.raw_source_ip() ^ ip_xor.rawNetwork());
				src_xor = true;
			}
			if (!dst_xor && 
					net->rawIsInNetwork(request.raw_destination_ip()))
			{
				request.raw_destination_ip(
					request.raw_destination_ip() ^ ip_xor.rawNetwork());
				dst_xor = true;

				// encrypt the host header
				request.host(encrypt(request.host()));
			}
		}

		// write the anonymized record
		output_writer.write(&request);
	}
	if (error_status != E_EOF) {
		cerr << argv[0] << ": error reading from '" << infile_name << "'" << endl;
		return 1;
	}

	if (reader.close() != E_SUCCESS) {
		cerr << argv[0] << ": error closing '" << infile_name << "'" << endl;
		return 1;
	}

	return 0;
}
