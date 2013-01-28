#include <iostream>
#include <iomanip>
#include <ios>
#include <ctime>

#include <boost/shared_ptr.hpp>

#include "PayloadSearchManagerArguments.h"
#include "configuration.hpp"
#include "hbf_query_processor.hpp"
#include "ostream_writer.hpp"
#include "fake_hbf.hpp"
#include "hbf_reader.hpp"
#include "vhbf_read_manager.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "FlatFileReader.hpp"
#include "data_type_traits/vhbf_header.hpp"
#include "data_type_traits/vhbf_column.hpp"

using namespace std;
using boost::shared_ptr;

ostream & operator << (ostream & os, const TimeStamp &t) {
	time_t _t(t.seconds());
	char str[20];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", gmtime(&_t));

	char oldfill(os.fill());
	ios_base::fmtflags oldflags(os.flags());
	streamsize oldwidth(os.width());
	os << str << '.' << setw(6) << setfill('0') << right << t.microseconds()
	   << " UTC" << endl;
	os.fill(oldfill);
	os.flags(oldflags);
	os.width(oldwidth);

	return os;
}

int main(int argc, char **argv) {
	PayloadSearchManagerArguments args(argc, argv);

	if (!args) {
		cerr << "Error: " << args.error() << endl << endl
			 << args << endl;

		return 1;
	}


	configuration conf;
	if (!conf.load(args.configFile().string())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}
	
	size_t max_mtu;
	if (conf.get(max_mtu, "max-mtu", "vhbf_query")
			!= configuration::OK)
	{
		cerr << argv[0] << ": max-mtu required" << endl;
		return 1;
	}

	size_t thread_count;
	if (conf.get(thread_count, "thread-count", "vhbf_query")
			!= configuration::OK)
	{
		cerr << argv[0] << ": thread-count required" << endl;
		return 1;
	}

	string data_directory(args.inputDir().string());
	TimeStamp start_time(args.startTime());
	TimeStamp end_time(args.endTime());

	typedef FlatFileReader<vhbf_header> header_reader_t;
	typedef FlatFileReader<vhbf_column> column_reader_t;
	typedef vhbf_read_manager<4 * 1024 * 8,
							  header_reader_t,
							  column_reader_t>
		vhbf_read_manager_t;
	typedef hbf_reader<vhbf_read_manager_t> hbf_reader_t;
	typedef ostream_writer<HBFResult> hbf_result_writer_t;

	shared_ptr<header_reader_t> header_reader(new header_reader_t());
	shared_ptr<column_reader_t> column_reader(new column_reader_t());
	hbf_result_writer_t writer(cout);

	StrftimeReadEnumerator header_enum(data_directory,
									   "%Y/%m/%d/vhbf_header_%H",
									   start_time,
									   end_time);
	if (header_enum.begin() == header_enum.end()) {
		cerr << "No files matching given time range" << endl;
		return 0;
	}

	StrftimeReadEnumerator column_enum(data_directory,
									   "%Y/%m/%d/vhbf_column_%H",
									   start_time,
									   end_time);
	if (column_enum.begin() == column_enum.end()) {
		cerr << "No files matching given time range" << endl;
		return 0;
	}

	if (distance(header_enum.begin(), header_enum.end())
			!= distance(column_enum.begin(), column_enum.end()))
	{
		cerr << "header/column file count mismatch." << endl;
		return 1;
	}

	for (StrftimeReadEnumerator::const_iterator i(header_enum.begin()),
		 j(column_enum.begin());
		 i != header_enum.end() || j != column_enum.end();
		 ++i, ++j)
	{
		string header_file(i->string());
		string column_file(j->string());
		if (header_file.substr(header_file.size() - 3)
				!= column_file.substr(column_file.size() - 3))
		{
			cerr << "header/column file time mismatch." << endl;
			return 1;
		}
	}


	shared_ptr<vhbf_read_manager_t> vrm;
	hbf_query_processor<hbf_reader_t, hbf_result_writer_t> qp;

	for (StrftimeReadEnumerator::const_iterator i(header_enum.begin()),
		 j(column_enum.begin());
		 i != header_enum.end() || j != column_enum.end();
		 ++i, ++j)
	{
		string header_file(i->string());
		string column_file(j->string());

		if (header_reader->open(header_file) != E_SUCCESS) {
			cerr << "Unable to open file '" << header_file << "'" << endl;
			return 1;
		}
		if (column_reader->open(column_file) != E_SUCCESS) {
			cerr << "Unable to open file '" << column_file << "'" << endl;
			return 1;
		}
		vrm.reset(new vhbf_read_manager_t(header_reader, column_reader));
		hbf_reader_t reader(vrm);
	
		qp.init(&reader,
				&writer,
				args.inputData(),
				args.queryLength(),
				args.matchLength(),
				args.ipv4FlowMatcher(),
				max_mtu,
				thread_count);

		if (qp.run() != 0) {
			cerr << "hbf_query_processor failed: " << qp.error() << endl;
			return 1;
		}


		if (header_reader->close() != E_SUCCESS) {
			cerr << "Unable to close file '" << header_file << "'" << endl;
			return 1;
		}
		if (column_reader->close() != E_SUCCESS) {
			cerr << "Unable to close file '" << column_file << "'" << endl;
			return 1;
		}
	}
	
	return 0;
}
