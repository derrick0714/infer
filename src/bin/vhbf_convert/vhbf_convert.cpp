#include <iostream>

#include <boost/program_options.hpp>

#include "vhbf_header.hpp"
#include "data_type_traits/vhbf_header.hpp"
#include "data_type_traits/vhbf_column.hpp"
#include "vhbf_chunk.hpp"
#include "fixed_time_functor.hpp"
#include "configuration.hpp"
#include "FlatFileReader.hpp"
#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "StrftimeWriteEnumerator.hpp"
#include "HBF.h"
#include "ZlibCompressedHBF.h"

using namespace std;
using namespace boost::program_options;

struct hbf_stats{
	hbf_stats()
		:hbf_count(0)
	{
		memset(&fill_rate_hist, 0, sizeof(fill_rate_hist));
	}

	size_t hbf_count;
	size_t fill_rate_hist[101];
};

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
	if (conf.get(data_directory, "data-directory", "vhbf_convert", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid data_directory" << endl;
		return 1;
	}
	
	string output_directory;
	if (conf.get(output_directory, "output-directory", "vhbf_convert")
			!= configuration::OK)
	{
		cerr << argv[0] << ": output-directory required" << endl;
		return 1;
	}

	fixed_time_functor start_time_functor(start_time);

	StrftimeReadEnumerator read_enum(data_directory,
									 "%Y/%m/%d/hbf_%H",
									 start_time,
									 start_time + TimeStamp(1, 0));
	if (read_enum.begin() == read_enum.end()) {
		cerr << "No files matching " << date << endl;
		return 1;
	}
	string zhbf_file(read_enum.begin()->string());

	boost::shared_ptr<StrftimeWriteEnumerator<vhbf_header, fixed_time_functor> >
		header_enumerator(new StrftimeWriteEnumerator<vhbf_header, fixed_time_functor>(
			output_directory, "%Y/%m/%d/vhbf_header_%H", start_time_functor));
	boost::shared_ptr<StrftimeWriteEnumerator<vhbf_column, fixed_time_functor> >
		column_enumerator(new StrftimeWriteEnumerator<vhbf_column, fixed_time_functor>(
			output_directory, "%Y/%m/%d/vhbf_column_%H", start_time_functor));

	InferFileWriter<FlatFileWriter<vhbf_header>, StrftimeWriteEnumerator<vhbf_header, fixed_time_functor> > header_writer(header_enumerator);
	InferFileWriter<FlatFileWriter<vhbf_column>, StrftimeWriteEnumerator<vhbf_column, fixed_time_functor> > column_writer(column_enumerator);

	FlatFileReader<ZlibCompressedHBF> reader;
	ZlibCompressedHBF zhbf;
	HBF hbf;
	vhbf_header header;
	vhbf_chunk<HBF::HBFSize> *chunk = new vhbf_chunk<HBF::HBFSize>;
	ErrorStatus error_status;

	if (reader.open(zhbf_file) != E_SUCCESS) {
		cerr << argv[0] << ": unable to open '" << zhbf_file << "'" << endl;
		return 1;
	}

	hbf_stats stats;
	size_t fill_rate;
	size_t count(0);
	while ((error_status = reader.read(zhbf)) == E_SUCCESS) {
		// do something
		if (hbf.init(zhbf) != Z_OK) {
			cerr << argv[0] << ": error decompressing hbf " << count
				 << " from '" << zhbf_file << "'" << endl;
			return 1;
		}

		// write header
		header.start_time(hbf.startTime());
		header.end_time(hbf.endTime());
		header.protocol(hbf.protocol());
		header.raw_source_ip(hbf.rawSourceIP());
		header.raw_destination_ip(hbf.rawDestinationIP());
		header.raw_source_port(hbf.rawSourcePort());
		header.raw_destination_port(hbf.rawDestinationPort());
		header.version(hbf.version());
		header.max_payload(hbf.maxPayload());
		header.num_insertions(hbf.numInsertions());
		
		++stats.hbf_count;
		fill_rate = static_cast<double>(hbf.numInsertions()) / (static_cast<double>(HBF::HBFSize) / 5) * 100;
		++stats.fill_rate_hist[(fill_rate > 100 ? 101 : fill_rate)];
		header_writer.write(&header);

		chunk->set_from_hbf(count, hbf.hbf());
		++count;
		if (count == chunk->column_height) {
			// write columns
			for (size_t i(0); i < chunk->column_count; ++i) {
				column_writer.write(&((*chunk)[i]));
			}

			count = 0;
			chunk->reset();
		}
	}
	if (error_status != E_EOF) {
		cerr << argv[0] << ": error reading from '" << zhbf_file << "'" << endl;
		return 1;
	}

	if (count != chunk->column_height) {
		// write final columns if they haven't been written yet
		for (size_t i(0); i < chunk->column_count; ++i) {
			column_writer.write(&((*chunk)[i]));
		}
	}
	if (reader.close() != E_SUCCESS) {
		cerr << argv[0] << ": error closing '" << zhbf_file << "'" << endl;
		return 1;
	}

	delete chunk;

	cout << stats.hbf_count << ' ';
	for (size_t i(0); i < sizeof(stats.fill_rate_hist) / sizeof(stats.fill_rate_hist[0]); ++i) {
		cout << stats.fill_rate_hist[i] << ' ';
	}
	cout << endl;

	return 0;
}
