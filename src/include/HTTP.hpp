#ifndef INFER_INCLUDE_HTTP_HPP_
#define INFER_INCLUDE_HTTP_HPP_

#include <netinet/in.h>

#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTP {
  public:
	HTTP()
		:_protocol(0),
		 _client_ip(0),
		 _server_ip(0),
		 _client_port(0),
		 _server_port(0),
		 _start_time(std::numeric_limits<uint32_t>::max(),
		 			 std::numeric_limits<uint32_t>::max()),
		 _end_time(0, 0),
		 _requests(),
		 _responses()
	{
	}

	HTTP(uint8_t protocol,
		 uint32_t client_ip,
		 uint32_t server_ip,
		 uint16_t client_port,
		 uint16_t server_port)
		:_protocol(protocol),
		 _client_ip(client_ip),
		 _server_ip(server_ip),
		 _client_port(client_port),
		 _server_port(server_port),
		 _start_time(std::numeric_limits<uint32_t>::max(),
		 			 std::numeric_limits<uint32_t>::max()),
		 _end_time(0, 0),
		 _requests(),
		 _responses()
	{
	}

	uint8_t protocol() const {
		return _protocol;
	}

	uint32_t client_ip() const {
		return ntohl(_client_ip);
	}

	uint32_t server_ip() const {
		return ntohl(_server_ip);
	}

	uint16_t client_port() const {
		return ntohs(_client_port);
	}

	uint16_t server_port() const {
		return ntohs(_server_port);
	}

	uint32_t raw_client_ip() const {
		return _client_ip;
	}

	uint32_t raw_server_ip() const {
		return _server_ip;
	}

	uint16_t raw_client_port() const {
		return _client_port;
	}

	uint16_t raw_server_port() const {
		return _server_port;
	}

	TimeStamp start_time() const {
		return _start_time;
	}

	TimeStamp end_time() const {
		return _end_time;
	}

	std::vector<HTTPRequest> requests() const {
		return _requests;
	}

	std::vector<HTTPResponse> responses() const {
		return _responses;
	}

	void protocol(uint8_t protocol) {
		_protocol = protocol;
	}

	void client_ip(uint32_t client_ip) {
		_client_ip = client_ip;
	}

	void server_ip(uint32_t server_ip) {
		_server_ip = server_ip;
	}

	void client_port(uint16_t client_port) {
		_client_port = client_port;
	}

	void server_port(uint16_t server_port) {
		_server_port = server_port;
	}

	void raw_client_ip(uint32_t client_ip) {
		_client_ip = client_ip;
	}

	void raw_server_ip(uint32_t server_ip) {
		_server_ip = server_ip;
	}

	void raw_client_port(uint16_t client_port) {
		_client_port = client_port;
	}

	void raw_server_port(uint16_t server_port) {
		_server_port = server_port;
	}

	void start_time(const TimeStamp &start_time) {
		_start_time = start_time;
	}

	void end_time(const TimeStamp &end_time) {
		_end_time = end_time;
	}

	void requests(const std::vector<HTTPRequest> &requests) {
		_requests = requests;
	}

	void responses(const std::vector<HTTPResponse> &responses) {
		_responses = responses;
	}

	void add_request(const HTTPRequest &http_request) {
		_requests.push_back(http_request);
		if (http_request.time() < _start_time) {
			_start_time = http_request.time();
		}
		if (http_request.time() > _end_time) {
			_end_time = http_request.time();
		}
	}

	void add_response(const HTTPResponse &http_response) {
		_responses.push_back(http_response);
		if (http_response.time() < _start_time) {
			_start_time = http_response.time();
		}
		if (http_response.time() > _end_time) {
			_end_time = http_response.time();
		}
	}

  private:
	uint8_t _protocol;
	uint32_t _client_ip;
	uint32_t _server_ip;
	uint16_t _client_port;
	uint16_t _server_port;
	TimeStamp _start_time;
	TimeStamp _end_time;
	std::vector<HTTPRequest> _requests;
	std::vector<HTTPResponse> _responses;
};

#endif
