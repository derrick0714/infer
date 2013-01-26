#include <iostream>
#include <string>
#include <cstring>
#include <queue>
#include <algorithm>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "modules.h"
#include "network.h"
#include "timeStamp.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "AsynchronousWriter.hpp"

#include "http_parser.h"

using namespace std;
using boost::lexical_cast;

static HTTPRequest *request;
static HTTPResponse *response;

//static AsynchronousWriter<InferFileWriter<FlatFileWriter<HTTP> > > *writer;
static AsynchronousWriter<InferFileWriter<FlatFileWriter<HTTPRequest> > >
		*request_writer;
static AsynchronousWriter<InferFileWriter<FlatFileWriter<HTTPResponse> > >
		*response_writer;
static http_parser _parser;
static http_parser_settings _parser_settings;

bool _got_url;
bool _headers_complete;
bool _message_complete;

struct parser_state {
	parser_state()
		:uri(),
		 cur_header(),
		 headers()
	{}

	void reset() {
		uri.clear();
		cur_header.clear();
		headers.clear();
	}

	string uri;
	string cur_header;
	map<string, string> headers;
};

parser_state _pstate;
const Packet *_packet;

// parser callbacks
int on_message_begin(http_parser *parser) {
	//cout << "on_message_begin() called..." << endl;

	_pstate.reset();

	return 0;
}

/*
int on_message_complete(http_parser *parser) {
	//cout << "on_message_complete() called..." << endl;

	_pstate.message_complete = true;

	return 0;
}
*/

int on_headers_complete(http_parser *parser) {
	//cout << "on_headers_complete() called..." << endl;

	if (_parser.type == HTTP_REQUEST) {
		// request
		/*
		cout << "HTTP Request:" << endl;

		cout << "Time:       " << _packet->time().seconds() << endl
			 << "Src IP:     " << ntohl(_packet->sourceIP()) << endl
			 << "Dest IP:    " << ntohl(_packet->destinationIP()) << endl
			 << "Src Port:   " << ntohs(_packet->sourcePort()) << endl
			 << "Dst Port:   " << ntohs(_packet->destinationPort()) << endl
			 << "Type:       " << http_method_str((http_method)parser->method)
				<< endl
			 << "URI:        " << _pstate.uri << endl
			 << "Version:    "
				<< "HTTP/" << _parser.http_major << '.' << _parser.http_minor
				<< endl
			 << "Host:       " << _pstate.headers["host"] << endl
			 << "User-Agent: " << _pstate.headers["user-agent"] << endl
			 << "Referer:    " << _pstate.headers["referer"] << endl;
		*/

		request = new HTTPRequest;
		request->time(_packet->time());
		request->raw_source_ip(_packet->sourceIP());
		request->raw_destination_ip(_packet->destinationIP());
		request->raw_source_port(_packet->sourcePort());
		request->raw_destination_port(_packet->destinationPort());
		request->type(http_method_str((http_method)parser->method));
		request->uri(_pstate.uri);
		request->version("HTTP/" + lexical_cast<string>(_parser.http_major) +
						 "." + lexical_cast<string>(_parser.http_minor));
		request->host(_pstate.headers["host"]);
		request->user_agent(_pstate.headers["user-agent"]);
		request->referer(_pstate.headers["referer"]);

		request_writer->write(request);
	}
	else if (_parser.type == HTTP_RESPONSE) {
		// response
		/*
		cout << "HTTP Response:" << endl;

		cout << "Time:       " << _packet->time().seconds() << endl
			 << "Src IP:     " << ntohl(_packet->sourceIP()) << endl
			 << "Dest IP:    " << ntohl(_packet->destinationIP()) << endl
			 << "Src Port:   " << ntohs(_packet->sourcePort()) << endl
			 << "Dst Port:   " << ntohs(_packet->destinationPort()) << endl
			 << "Version:    "
				<< "HTTP/" << _parser.http_major << '.' << _parser.http_minor
				<< endl
			 << "Status:     " << _parser.status_code << endl
			 << "Server:     " << _pstate.headers["server"] << endl
			 << "Content:    " << _pstate.headers["content-type"] << endl;
		*/

		response = new HTTPResponse;
		response->time(_packet->time());
		response->raw_source_ip(_packet->sourceIP());
		response->raw_destination_ip(_packet->destinationIP());
		response->raw_source_port(_packet->sourcePort());
		response->raw_destination_port(_packet->destinationPort());
		response->version("HTTP/" + lexical_cast<string>(_parser.http_major) +
						 "." + lexical_cast<string>(_parser.http_minor));
		response->status(lexical_cast<string>(_parser.status_code));
		response->response(_pstate.headers["server"]);
		response->content_type(_pstate.headers["content-type"]);

		response_writer->write(response);
	}

	//cout << endl;

	return 0;
}

/*
int on_path(http_parser *parser, const char *at, size_t len) {
	cout << "on_path(): " << string(at, len) << endl;

	return 0;
}

int on_query_string(http_parser *parser, const char *at, size_t len) {
	cout << "on_query_string(): " << string(at, len) << endl;

	return 0;
}
*/

int on_url(http_parser *parser, const char *at, size_t len) {
	//_pstate.method.assign(http_method_str((http_method)parser->method));
	_pstate.uri.assign(at, len);

	return 0;
}

int on_header_field(http_parser *parser, const char *at, size_t len) {
	//cout << "on_header_field() called..." << endl;

	// since we only look at single packets, we don't need to check if the
	// entire field name is given. Assume it is, and if not, we'll never see
	// the rest of it anyway, so it doesn't matter.
	_pstate.cur_header.assign(at, len);
	transform(_pstate.cur_header.begin(),
			  _pstate.cur_header.end(),
			  _pstate.cur_header.begin(),
			  (int (*)(int)) tolower);

	return 0;
}

int on_header_value(http_parser *parser, const char *at, size_t len) {
	//cout << "on_header_value() called..." << endl;
	_pstate.headers[_pstate.cur_header].assign(at, len);

	return 0;
}

extern "C" {

	int initialize(const configuration &,
				   const std::string &outputDirectory,
				   const std::string &)
	{
		boost::shared_ptr<StrftimeWriteEnumerator<HTTPRequest> >
				request_enumerator(new StrftimeWriteEnumerator<HTTPRequest>(
						outputDirectory, "%Y/%m/%d/http_request_%H"));
		boost::shared_ptr<StrftimeWriteEnumerator<HTTPResponse> >
				response_enumerator(new StrftimeWriteEnumerator<HTTPResponse>(
						outputDirectory, "%Y/%m/%d/http_response_%H"));

		boost::shared_ptr<InferFileWriter<FlatFileWriter<HTTPRequest> > >
			request_infer_writer(
				new InferFileWriter<FlatFileWriter<HTTPRequest> >(
					request_enumerator));
		request_writer = new AsynchronousWriter
							<InferFileWriter<FlatFileWriter<HTTPRequest> > >
								(request_infer_writer);

		boost::shared_ptr<InferFileWriter<FlatFileWriter<HTTPResponse> > >
			response_infer_writer(
				new InferFileWriter<FlatFileWriter<HTTPResponse> >(
					response_enumerator));
		response_writer = new AsynchronousWriter
							<InferFileWriter<FlatFileWriter<HTTPResponse> > >
								(response_infer_writer);

		// http parser...

		_parser_settings.on_message_begin = on_message_begin;
		//_parser_settings.on_path = on_path;
		//_parser_settings.on_query_string = on_query_string;
		_parser_settings.on_url = on_url;
		// on_fragment
		_parser_settings.on_header_field = on_header_field;
		_parser_settings.on_header_value = on_header_value;
		_parser_settings.on_headers_complete = on_headers_complete;
		// on_body
		//_parser_settings.on_message_complete = on_message_complete;

		return 0;
	}

	int processPacket(Packet &packet) {
		static size_t nparsed;

		_packet = &packet;
		http_parser_init(&_parser, HTTP_BOTH);


		nparsed = http_parser_execute(&_parser,
									  &_parser_settings,
									  packet.payload(),
									  packet.payloadSize());

		/*
		if (_got_url) {
			if (!_pstate.headers_complete) {
				cout << "WARN: Headers incomplete!" << endl;
			}

			if (!_pstate.message_complete) {
				cout << "WARN: Message incomplete!" << endl;
			}
		}

		if (nparsed != packet.payloadSize()) {
			cout << "WARN: entire packet not parsed!" << endl;
		}

		cout << endl << endl;
		*/

		return 0;
	}

	int flush() {
		return 0;
	}

	int finish() {
		request_writer->close();
		response_writer->close();
		delete request_writer;
		delete response_writer;
		return 0;
	}
}
