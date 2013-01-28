#include <iostream>
#include <ios>
#include <iomanip>
#include <ctime>

#include <boost/program_options.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include "configuration.hpp"
#include "fake_hbf.hpp"
#include "hbf_reader.hpp"
#include "vhbf_read_manager.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "FlatFileReader.hpp"
#include "data_type_traits/vhbf_header.hpp"
#include "data_type_traits/vhbf_column.hpp"

using namespace std;
using namespace boost::program_options;
using boost::shared_ptr;

ostream & operator << (ostream & os, const TimeStamp &t) {
	time_t _t(t.seconds());
	char str[20];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", gmtime(&_t));

	char oldfill(os.fill());
	ios_base::fmtflags oldflags(os.flags());
	streamsize oldwidth(os.width());
	os << str << '.' << setw(6) << setfill('0') << right << t.microseconds();
	os.fill(oldfill);
	os.flags(oldflags);
	os.width(oldwidth);

	return os;
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
	if (conf.get(data_directory, "data-directory", "vhbf_test", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid data_directory" << endl;
		return 1;
	}

	StrftimeReadEnumerator header_enum(data_directory,
									   "%Y/%m/%d/vhbf_header_%H",
									   start_time,
									   start_time + TimeStamp(1, 0));
	if (header_enum.begin() == header_enum.end()) {
		cerr << "No files matching " << date << endl;
		return 1;
	}
	string header_file(header_enum.begin()->string());

	StrftimeReadEnumerator column_enum(data_directory,
									   "%Y/%m/%d/vhbf_column_%H",
									   start_time,
									   start_time + TimeStamp(1, 0));
	if (column_enum.begin() == column_enum.end()) {
		cerr << "No files matching " << date << endl;
		return 1;
	}
	string column_file(column_enum.begin()->string());

	typedef FlatFileReader<vhbf_header> header_reader_t;
	typedef FlatFileReader<vhbf_column> column_reader_t;
	typedef vhbf_read_manager<4 * 1024 * 8,
							  header_reader_t,
							  column_reader_t>
		vhbf_read_manager_t;

	shared_ptr<header_reader_t> header_reader(new header_reader_t());
	shared_ptr<column_reader_t> column_reader(new column_reader_t());

	if (header_reader->open(header_file) != E_SUCCESS) {
		cerr << "Unable to open file '" << header_file << "'" << endl;
		return 1;
	}
	if (column_reader->open(column_file) != E_SUCCESS) {
		cerr << "Unable to open file '" << column_file << "'" << endl;
		return 1;
	}
	
	fake_hbf<vhbf_read_manager_t> hbf;
	ErrorStatus error_status;

	shared_ptr<vhbf_read_manager_t> vrm(
		new vhbf_read_manager_t(header_reader, column_reader));
	hbf_reader<vhbf_read_manager_t> reader(vrm);
	
	size_t count(0);
	while ((error_status = reader.read(hbf)) == E_SUCCESS) {
		using namespace boost::asio;

		cout << setw(24) << left << "Start Time:"
			 << hbf.start_time() << endl;
		cout << setw(24) << left << "End Time:"
			 << hbf.end_time() << endl;
		cout << setw(24) << left << "Source IP:"
			 << ip::address_v4(hbf.source_ip()) << endl;
		cout << setw(24) << left << "Destination IP:"
			 << ip::address_v4(hbf.destination_ip()) << endl;
		cout << setw(24) << left << "Source Port:"
			 << hbf.source_port() << endl;
		cout << setw(24) << left << "Destination Port:"
			 << hbf.destination_port() << endl;
		cout << setw(24) << left << "Max Payload:"
			 << hbf.max_payload() << endl;
		cout << setw(24) << left << "Number of Insertions:"
			 << hbf.num_insertions() << endl;
		cout << endl;

		hbf.test(count % hbf.hbf_size);

		++count;
	}

	cerr << "Records read: " << count << endl;
	cerr << "count * hbf.hbf_size / 8: " << count * hbf.hbf_size / 8 << endl;

	return 0;
}
