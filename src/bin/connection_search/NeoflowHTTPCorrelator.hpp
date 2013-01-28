#ifndef INFER_BIN_CONNECTIONSEARCH_NEOFLOWHTTPCORRELATOR_HPP_
#define INFER_BIN_CONNECTIONSEARCH_NEOFLOWHTTPCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>
#include <string>
#include <tr1/unordered_map>

#include "ConnectionSearchConnection.hpp"
#include "ConnectionSearchNeoflow.hpp"
#include "ConnectionSearchHTTP.hpp"
#include "ConnectionSearchHTTPRequest.hpp"
#include "ConnectionSearchHTTPResponse.hpp"
#include "ConnectionSearchHTTPRequestReference.hpp"
#include "ConnectionSearchHTTPResponseReference.hpp"
#include "ConnectionSearchConnectionHTTPReference.hpp"

#include "HTTP.hpp"
#include "IPv4Network.hpp"
#include "IPv4FlowMatcher.hpp"
#include "timeStamp.h"
#include "ErrorStatus.hpp"

template <typename RequestReaderType,
		  typename ResponseReaderType,
		  typename HTTPWriterType,
		  typename HTTPRequestWriterType,
		  typename HTTPResponseWriterType,
		  typename HTTPRequestRefWriterType,
		  typename HTTPResponseRefWriterType,
		  typename ConnectionHTTPRefWriterType>
class NeoflowHTTPCorrelator {
  public:
	NeoflowHTTPCorrelator()
		:_request_reader(NULL),
		 _response_reader(NULL),
		 _http_writer(NULL),
		 _request_writer(NULL),
		 _response_writer(NULL),
		 _request_ref_writer(NULL),
		 _response_ref_writer(NULL),
		 _conn_http_ref_writer(NULL),
		 _set(NULL),
		 _error(false),
		 _errorMsg(),
		 _http_id(0),
		 _http_request_id(0),
		 _http_response_id(0)
	{
		check_request_reader_type(typename RequestReaderType::value_type());
		check_response_reader_type(typename ResponseReaderType::value_type());
		check_http_writer_type(typename HTTPWriterType::value_type());
		check_request_writer_type(typename HTTPRequestWriterType::value_type());
		check_response_writer_type(typename HTTPResponseWriterType::value_type());
		check_request_ref_writer_type(typename HTTPRequestRefWriterType::value_type());
		check_response_ref_writer_type(typename HTTPResponseRefWriterType::value_type());
		check_conn_http_ref_writer_type(typename ConnectionHTTPRefWriterType::value_type());
	}

	NeoflowHTTPCorrelator(RequestReaderType *request_reader,
						ResponseReaderType *response_reader,
						HTTPWriterType *http_writer,
						HTTPRequestWriterType *request_writer,
						HTTPResponseWriterType *response_writer,
						HTTPRequestRefWriterType *request_ref_writer,
						HTTPResponseRefWriterType *response_ref_writer,
						ConnectionHTTPRefWriterType *conn_http_ref_writer,
						const std::set <ConnectionSearchConnection> *set)
		:_request_reader(request_reader),
		 _response_reader(response_reader),
		 _http_writer(http_writer),
		 _request_writer(request_writer),
		 _response_writer(response_writer),
		 _request_ref_writer(request_ref_writer),
		 _response_ref_writer(response_ref_writer),
		 _conn_http_ref_writer(conn_http_ref_writer),
		 _set(set),
		 _error(false),
		 _errorMsg(),
		 _http_id(0),
		 _http_request_id(0),
		 _http_response_id(0)
	{
		check_request_reader_type(typename RequestReaderType::value_type());
		check_response_reader_type(typename ResponseReaderType::value_type());
		check_http_writer_type(typename HTTPWriterType::value_type());
		check_request_writer_type(typename HTTPRequestWriterType::value_type());
		check_response_writer_type(typename HTTPResponseWriterType::value_type());
		check_request_ref_writer_type(typename HTTPRequestRefWriterType::value_type());
		check_response_ref_writer_type(typename HTTPResponseRefWriterType::value_type());
		check_conn_http_ref_writer_type(typename ConnectionHTTPRefWriterType::value_type());
	}

	void init(RequestReaderType *request_reader,
			  ResponseReaderType *response_reader,
			  HTTPWriterType *http_writer,
			  HTTPRequestWriterType *request_writer,
			  HTTPResponseWriterType *response_writer,
			  HTTPRequestRefWriterType *request_ref_writer,
			  HTTPResponseRefWriterType *response_ref_writer,
			  ConnectionHTTPRefWriterType *conn_http_ref_writer,
			  std::set <ConnectionSearchConnection> *set)
	{
		_request_reader = request_reader;
		_response_reader = response_reader;
		_http_writer = http_writer;
		_request_writer = request_writer;
		_response_writer = response_writer;
		_request_ref_writer = request_ref_writer;
		_response_ref_writer = response_ref_writer;
		_conn_http_ref_writer = conn_http_ref_writer;
		_set = set;
	}

	int run();

	std::string error() const {
		return _errorMsg;
	}

  private:
	NeoflowHTTPCorrelator(const NeoflowHTTPCorrelator &);
	const NeoflowHTTPCorrelator & operator=(const NeoflowHTTPCorrelator &);

	void check_request_reader_type(HTTPRequest) {}
	void check_response_reader_type(HTTPResponse) {}
	void check_http_writer_type(ConnectionSearchHTTP) {}
	void check_request_writer_type(ConnectionSearchHTTPRequest) {}
	void check_response_writer_type(ConnectionSearchHTTPResponse) {}
	void check_request_ref_writer_type(ConnectionSearchHTTPRequestReference) {}
	void check_response_ref_writer_type(ConnectionSearchHTTPResponseReference) {}
	void check_conn_http_ref_writer_type(ConnectionSearchConnectionHTTPReference) {}

	RequestReaderType *_request_reader;
	ResponseReaderType *_response_reader;
	HTTPWriterType *_http_writer;
	HTTPRequestWriterType *_request_writer;
	HTTPResponseWriterType *_response_writer;
	HTTPRequestRefWriterType *_request_ref_writer;
	HTTPResponseRefWriterType *_response_ref_writer;
	ConnectionHTTPRefWriterType *_conn_http_ref_writer;
	const std::set <ConnectionSearchConnection> *_set;

	bool _error;
	std::string _errorMsg;

	uint32_t _http_id;
	uint32_t _http_request_id;
	uint32_t _http_response_id;
};

template <typename RequestReaderType,
		  typename ResponseReaderType,
		  typename HTTPWriterType,
		  typename HTTPRequestWriterType,
		  typename HTTPResponseWriterType,
		  typename HTTPRequestRefWriterType,
		  typename HTTPResponseRefWriterType,
		  typename ConnectionHTTPRefWriterType>
int NeoflowHTTPCorrelator<RequestReaderType,
						  ResponseReaderType,
						  HTTPWriterType,
						  HTTPRequestWriterType,
						  HTTPResponseWriterType,
						  HTTPRequestRefWriterType,
						  HTTPResponseRefWriterType,
						  ConnectionHTTPRefWriterType>
::run() {
	HTTPRequest http_request;
	HTTPResponse http_response;
	IPv4Network network;
	ConnectionSearchHTTP connection_search_http;
	std::tr1::unordered_map<std::string, HTTP> http_map;
	uint8_t id_protocol;
	uint32_t id_src_ip;
	uint32_t id_dst_ip;
	uint16_t id_src_port;
	uint16_t id_dst_port;
	ConnectionSearchHTTP connectionSearchHTTP;
	ConnectionSearchHTTPRequest requestRow;
	ConnectionSearchHTTPResponse responseRow;
	ConnectionSearchHTTPRequestReference requestRef;
	ConnectionSearchHTTPResponseReference responseRef;
	ConnectionSearchConnectionHTTPReference connHTTPRef;
	ErrorStatus errorStatus;
	std::string httpFlowID;

	// fill up the http_map
	while ((errorStatus = _request_reader->read(http_request)) == E_SUCCESS) {
		// formulate the map key
		id_protocol = http_request.protocol();
		httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
						  sizeof(id_protocol));
		id_src_ip = http_request.raw_source_ip();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
						  sizeof(id_src_ip));
		id_dst_ip = http_request.raw_destination_ip();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
						  sizeof(id_dst_ip));
		id_src_port = http_request.raw_source_port();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
						  sizeof(id_src_port));
		id_dst_port = http_request.raw_destination_port();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
						  sizeof(id_dst_port));

		// check if there's an HTTP already in the map
		//      if not, create one
		std::tr1::unordered_map<std::string, HTTP>::iterator httpIt(http_map.find(httpFlowID));
		if (httpIt == http_map.end()) {
			httpIt = http_map.insert(std::make_pair(httpFlowID,
											  HTTP(id_protocol,
												   id_src_ip,
												   id_dst_ip,
												   id_src_port,
												   id_dst_port))).first;
		}

		// add this http_request to the HTTP in the map
		httpIt->second.add_request(http_request);
	}
	if (errorStatus != E_EOF) {
		_error = true;
		_errorMsg.assign("Reader: error reading http_request");
		return 1;
	}

	while ((errorStatus = _response_reader->read(http_response)) == E_SUCCESS) {
		// formulate the map key
		id_protocol = http_response.protocol();
		httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
						  sizeof(id_protocol));
		id_dst_ip = http_response.raw_destination_ip();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
						  sizeof(id_dst_ip));
		id_src_ip = http_response.raw_source_ip();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
						  sizeof(id_src_ip));
		id_dst_port = http_response.raw_destination_port();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
						  sizeof(id_dst_port));
		id_src_port = http_response.raw_source_port();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
						  sizeof(id_src_port));

		std::tr1::unordered_map<std::string, HTTP>::iterator httpIt(http_map.find(httpFlowID));
		if (httpIt != http_map.end()) {
			// add it
			httpIt->second.add_response(http_response);
		}
		httpIt = http_map.insert(make_pair(httpFlowID,
										  HTTP(id_protocol,
											   id_dst_ip,
											   id_src_ip,
											   id_dst_port,
											   id_src_port))).first;

		// add this http_response to the HTTP in the map
		httpIt->second.add_response(http_response);
	}
	if (errorStatus != E_EOF) {
		_error = true;
		_errorMsg.assign("Reader: error reading response");
		return 1;
	}

	// for each Connection
	for (std::set<ConnectionSearchConnection>::const_iterator i(_set->begin());
		 i != _set->end();
		 ++i)
	{
		// create the flowID
		id_protocol = i->protocol();
		httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
						  sizeof(id_protocol));
		id_dst_ip = i->raw_ip_a();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
						  sizeof(id_dst_ip));
		id_src_ip = i->raw_ip_b();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
						  sizeof(id_src_ip));
		id_dst_port = i->raw_port_a();
		httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
						  sizeof(id_dst_port));
		id_src_port = i->raw_port_b();
		httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
						  sizeof(id_src_port));
		// check for flow ID
		std::tr1::unordered_map<std::string, HTTP>::iterator httpIt(http_map.find(httpFlowID));
		if (httpIt == http_map.end()) {
			// if not found, create reverse flowID
			httpFlowID.assign(reinterpret_cast<char *>(&id_protocol),
							  sizeof(id_protocol));
			id_dst_ip = i->raw_ip_b();
			httpFlowID.append(reinterpret_cast<char *>(&id_dst_ip),
							  sizeof(id_dst_ip));
			id_src_ip = i->raw_ip_a();
			httpFlowID.append(reinterpret_cast<char *>(&id_src_ip),
							  sizeof(id_src_ip));
			id_dst_port = i->raw_port_b();
			httpFlowID.append(reinterpret_cast<char *>(&id_dst_port),
							  sizeof(id_dst_port));
			id_src_port = i->raw_port_a();
			httpFlowID.append(reinterpret_cast<char *>(&id_src_port),
							  sizeof(id_src_port));

			httpIt = http_map.find(httpFlowID);
			if (httpIt == http_map.end()) {
				continue;
			}
		}

		// found...write http and xref info
		connectionSearchHTTP.http_id(_http_id);
		connectionSearchHTTP.start_time(httpIt->second.start_time());
		connectionSearchHTTP.end_time(httpIt->second.end_time());
		connectionSearchHTTP.protocol(httpIt->second.protocol());
		connectionSearchHTTP.client_ip(httpIt->second.client_ip());
		connectionSearchHTTP.server_ip(httpIt->second.server_ip());
		connectionSearchHTTP.client_port(httpIt->second.client_port());
		connectionSearchHTTP.server_port(httpIt->second.server_port());

		_http_writer->write(connectionSearchHTTP);

		connHTTPRef.http_id(_http_id);
		connHTTPRef.connection_id(i->connection_id());

		_conn_http_ref_writer->write(connHTTPRef);

		// write the requests
		std::vector<HTTPRequest> requests(httpIt->second.requests());
		requestRef.http_id(_http_id);
		for (std::vector<HTTPRequest>::const_iterator req(requests.begin());
			 req != requests.end();
			 ++req)
		{
			requestRow.http_request_id(_http_request_id);
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

			_request_writer->write(requestRow);

			requestRef.http_request_id(_http_request_id);

			_request_ref_writer->write(requestRef);

			++_http_request_id;
		}

		// write the responses
		std::vector<HTTPResponse> responses(httpIt->second.responses());
		responseRef.http_id(_http_id);
		for (std::vector<HTTPResponse>::const_iterator res(responses.begin());
			 res != responses.end();
			 ++res)
		{
			responseRow.http_response_id(_http_response_id);
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

			_response_writer->write(responseRow);

			responseRef.http_response_id(_http_response_id);

			_response_ref_writer->write(responseRef);

			++_http_response_id;
		}

		++_http_id;
	}

	return 0;
}

#endif
