#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "FlatFileReader.hpp"
#include "EnumeratedFileReader.hpp"
#include "StrftimeReadEnumerator.hpp"
#include "oldHTTP.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "InferFileWriter.hpp"
#include "FlatFileWriter.hpp"
#include "configuration.hpp"

using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file", 
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		("date", value<string>(), "the date (YYYY-mm-dd)")
	;

	positional_options_description p;
	p.add("date", 1);
	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).positional(p).run(), vm);
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

	if (!vm.count("date")) {
		cerr << "Error: date is requred." << endl;
		cerr << desc_gen << endl;
		return 1;
	}

	struct tm _tm;
	if (strptime(vm["date"].as<string>().c_str(), "%Y-%m-%d", &_tm) == NULL) {
		cerr << "Error: invalid date: " << vm["date"].as<string>() << endl;
		return 1;
	}

	cerr << "Config file: " << vm["config-file"].as<string>().c_str() << endl;
	cerr << "Date:        " << vm["date"].as<string>().c_str() << endl;

	struct tm tm_begin, tm_end;
	memset(&tm_begin, 0, sizeof(tm_begin));
	memset(&tm_end, 0, sizeof(tm_end));

	tm_begin.tm_isdst = tm_end.tm_isdst = -1;
	tm_begin.tm_year = tm_end.tm_year = _tm.tm_year;
	tm_begin.tm_mon = tm_end.tm_mon = _tm.tm_mon;
	tm_begin.tm_mday = tm_end.tm_mday = _tm.tm_mday;
	tm_begin.tm_hour = 0;
	tm_end.tm_hour = 24;

	time_t unix_begin, unix_end;
	unix_begin = mktime(&tm_begin);
	unix_end = mktime(&tm_end);

	TimeStamp time_begin, time_end;
	time_begin.set(unix_begin, 0);
	time_end.set(unix_end, 0);

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}
	
	string data_directory;
	if (conf.get(data_directory, "data-directory", "split_http", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string output_directory;
	if (conf.get(output_directory, "output-directory", "split_http")
			!= configuration::OK)
	{
		cerr << argv[0] << ": output-directory required" << endl;
		return 1;
	}


	OldHTTP http;
	HTTPRequest request;
	HTTPResponse response;

	boost::shared_ptr<StrftimeReadEnumerator> readEnum(
		new StrftimeReadEnumerator);

	readEnum->init(data_directory,
				   "%Y/%m/%d/http_%H",
				   time_begin,
				   time_end);
	if (!(*readEnum)) {
		cerr << "Error: Unable to initialize read readEnumerator" << endl;
		return 1;
	}

	EnumeratedFileReader
		<FlatFileReader
			<OldHTTP>,
		 StrftimeReadEnumerator
		> reader;
	ErrorStatus error_status;
	error_status = reader.init(readEnum);
	if (error_status != E_SUCCESS && error_status != E_EOF) {
		cerr << "Error: Unable to initialize EnumeratedFileReader" << endl;
		return 1;
	}

	boost::shared_ptr<StrftimeWriteEnumerator<HTTPRequest> >
		request_enumerator(new StrftimeWriteEnumerator<HTTPRequest>(
			output_directory, "%Y/%m/%d/http_request_%H"));
	boost::shared_ptr<StrftimeWriteEnumerator<HTTPResponse> >
		response_enumerator(new StrftimeWriteEnumerator<HTTPResponse>(
			output_directory, "%Y/%m/%d/http_response_%H"));

	InferFileWriter<FlatFileWriter<HTTPRequest> > request_writer(request_enumerator);

	InferFileWriter<FlatFileWriter<HTTPResponse> > response_writer(response_enumerator);

	while ((error_status = reader.read(http)) == E_SUCCESS) {
		if (http.type() == 'q') {
			request.time(http.time());
			request.protocol(http.protocol());
			request.raw_source_ip(http.rawSourceIP());
			request.raw_destination_ip(http.rawDestinationIP());
			request.raw_source_port(http.rawSourcePort());
			request.raw_destination_port(http.rawDestinationPort());

			request.type(http.requestType());
			request.uri(http.uri());
			request.version(http.version());
			request.host(http.host());
			request.user_agent(http.userAgent());
			request.referer(http.referer());

			request_writer.write(&request);
		}
		else {
			response.time(http.time());
			response.protocol(http.protocol());
			response.raw_source_ip(http.rawSourceIP());
			response.raw_destination_ip(http.rawDestinationIP());
			response.raw_source_port(http.rawSourcePort());
			response.raw_destination_port(http.rawDestinationPort());

			response.version(http.version());
			response.status(http.status());
			response.reason(http.reason());
			response.response(http.response());
			response.content_type(http.contentType());

			response_writer.write(&response);
		}
	}
	if (error_status != E_EOF) {
		cerr << argv[0] << ": error reading HTTP data!" << endl;
		return 1;
	}

	if (request_writer.close() != E_SUCCESS) {
		cerr << "Error closing request_writer" << endl;
		return 1;
	}

	if (response_writer.close() != E_SUCCESS) {
		cerr << "Error closing response_writer" << endl;
		return 1;
	}

	return 0;
}
