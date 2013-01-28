#include <iostream>
#include <iomanip>
#include <ios>
#include <ctime>

#include <boost/shared_ptr.hpp>

#include "PayloadSearchManagerArguments.h"
#include "configuration.hpp"
#include "hbf_query_processor.hpp"
#include "ostream_writer.hpp"
#include "hbf.hpp"
#include "hbf_reader.hpp"
#include "zhbf_read_manager.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "FlatFileReader.hpp"

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
	if (conf.get(max_mtu, "max-mtu", "hbf_query")
			!= configuration::OK)
	{
		cerr << argv[0] << ": max-mtu required" << endl;
		return 1;
	}

	size_t thread_count;
	if (conf.get(thread_count, "thread-count", "hbf_query")
			!= configuration::OK)
	{
		cerr << argv[0] << ": thread-count required" << endl;
		return 1;
	}

	string data_directory(args.inputDir().string());
	TimeStamp start_time(args.startTime());
	TimeStamp end_time(args.endTime());

	typedef FlatFileReader<ZlibCompressedHBF> zhbf_reader_t;
	typedef zhbf_read_manager<zhbf_reader_t> hbf_read_manager_t;
	typedef hbf_reader<hbf_read_manager_t> hbf_reader_t;
	typedef ostream_writer<HBFResult> hbf_result_writer_t;

	shared_ptr<zhbf_reader_t> zhbf_reader(new zhbf_reader_t());
	hbf_result_writer_t writer(cout);

	StrftimeReadEnumerator zhbf_enum(data_directory,
									   "%Y/%m/%d/hbf_%H",
									   start_time,
									   end_time);
	if (zhbf_enum.begin() == zhbf_enum.end()) {
		cerr << "No files matching given time range" << endl;
		return 0;
	}

	shared_ptr<hbf_read_manager_t> zrm;
	hbf_query_processor<hbf_reader_t, hbf_result_writer_t> qp;

	for (StrftimeReadEnumerator::const_iterator i(zhbf_enum.begin());
		 i != zhbf_enum.end();
		 ++i)
	{
		string zhbf_file(i->string());

		if (zhbf_reader->open(zhbf_file) != E_SUCCESS) {
			cerr << "Unable to open file '" << zhbf_file << "'" << endl;
			return 1;
		}
		zrm.reset(new hbf_read_manager_t(zhbf_reader));
		hbf_reader_t reader(zrm);
	
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

		if (zhbf_reader->close() != E_SUCCESS) {
			cerr << "Unable to close file '" << zhbf_file << "'" << endl;
			return 1;
		}
	}
	
	return 0;
}
