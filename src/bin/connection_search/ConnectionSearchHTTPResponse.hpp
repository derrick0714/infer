#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPRESPONSE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPRESPONSE_HPP_

#include "timeStamp.h"

class ConnectionSearchHTTPResponse {
  public:
	ConnectionSearchHTTPResponse()
		:_http_response_id(0),
		 _version(),
		 _status(),
		 _reason(),
		 _response(),
		 _content_type()
	{
	}

	uint32_t http_response_id() const;
	uint8_t protocol() const;
	uint32_t source_ip() const;
	uint32_t destination_ip() const;
	uint32_t source_port() const;
	uint32_t destination_port() const;
	TimeStamp time() const;
	std::string version() const;
	std::string status() const;
	std::string reason() const;
	std::string response() const;
	std::string content_type() const;

	void http_response_id(uint32_t http_response_id);
	void protocol(uint8_t protocol);
	void source_ip(uint32_t source_ip);
	void destination_ip(uint32_t destination_ip);
	void source_port(uint32_t source_port);
	void destination_port(uint32_t destination_port);
	void time(const TimeStamp &time);
	void version(const std::string &version);
	void status(const std::string &status);
	void reason(const std::string &reason);
	void response(const std::string &response);
	void content_type(const std::string &content_type);

	bool operator<(const ConnectionSearchHTTPResponse &rhs) const {
		return _time < rhs._time;
	}

  private:
	uint32_t _http_response_id;
	TimeStamp _time;
	uint8_t _protocol;
	uint32_t _source_ip;
	uint32_t _destination_ip;
	uint32_t _source_port;
	uint32_t _destination_port;
	std::string _version;
	std::string _status;
	std::string _reason;
	std::string _response;
	std::string _content_type;
};

inline uint32_t ConnectionSearchHTTPResponse::http_response_id() const {
	return _http_response_id;
}

inline uint8_t ConnectionSearchHTTPResponse::protocol() const {
	return _protocol;
}

inline uint32_t ConnectionSearchHTTPResponse::source_ip() const {
	return _source_ip;
}

inline uint32_t ConnectionSearchHTTPResponse::destination_ip() const {
	return _destination_ip;
}

inline uint32_t ConnectionSearchHTTPResponse::source_port() const {
	return _source_port;
}

inline uint32_t ConnectionSearchHTTPResponse::destination_port() const {
	return _destination_port;
}

inline TimeStamp ConnectionSearchHTTPResponse::time() const {
	return _time;
}

inline std::string ConnectionSearchHTTPResponse::version() const {
	return _version;
}

inline std::string ConnectionSearchHTTPResponse::status() const {
	return _status;
}

inline std::string ConnectionSearchHTTPResponse::reason() const {
	return _reason;
}

inline std::string ConnectionSearchHTTPResponse::response() const {
	return _response;
}

inline std::string ConnectionSearchHTTPResponse::content_type() const {
	return _content_type;
}

inline void ConnectionSearchHTTPResponse::http_response_id(uint32_t http_response_id) {
	_http_response_id = http_response_id;
}

inline void ConnectionSearchHTTPResponse::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void ConnectionSearchHTTPResponse::source_ip(uint32_t source_ip) {
	_source_ip = source_ip;
}

inline void ConnectionSearchHTTPResponse::destination_ip(uint32_t destination_ip) {
	_destination_ip = destination_ip;
}

inline void ConnectionSearchHTTPResponse::source_port(uint32_t source_port) {
	_source_port = source_port;
}

inline void ConnectionSearchHTTPResponse::destination_port(uint32_t destination_port) {
	_destination_port = destination_port;
}

inline void ConnectionSearchHTTPResponse::time(const TimeStamp &time) {
	_time = time;
}

inline void ConnectionSearchHTTPResponse::version(const std::string &version) {
	_version = version;
}

inline void ConnectionSearchHTTPResponse::status(const std::string &status) {
	_status = status;
}

inline void ConnectionSearchHTTPResponse::reason(const std::string &reason) {
	_reason = reason;
}

inline void ConnectionSearchHTTPResponse::response(const std::string &response) {
	_response = response;
}

inline void ConnectionSearchHTTPResponse::content_type(const std::string &content_type) {
	_content_type = content_type;
}

#endif
