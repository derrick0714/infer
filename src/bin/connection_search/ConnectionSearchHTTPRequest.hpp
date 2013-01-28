#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPREQUEST_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPREQUEST_HPP_

#include "timeStamp.h"

class ConnectionSearchHTTPRequest {
  public:
	ConnectionSearchHTTPRequest()
		:_http_request_id(0),
		 _time(),
		 _type(),
		 _uri(),
		 _version(),
		 _host(),
		 _user_agent(),
		 _referer()
	{
	}

	uint32_t http_request_id() const;
	uint8_t protocol() const;
	uint32_t source_ip() const;
	uint32_t destination_ip() const;
	uint32_t source_port() const;
	uint32_t destination_port() const;
	TimeStamp time() const;
	std::string type() const;
	std::string uri() const;
	std::string version() const;
	std::string host() const;
	std::string user_agent() const;
	std::string referer() const;

	void http_request_id(uint32_t http_request_id);
	void protocol(uint8_t protocol);
	void source_ip(uint32_t source_ip);
	void destination_ip(uint32_t destination_ip);
	void source_port(uint32_t source_port);
	void destination_port(uint32_t destination_port);
	void time(const TimeStamp &time);
	void type(const std::string &type);
	void uri(const std::string &uri);
	void version(const std::string &version);
	void host(const std::string &host);
	void user_agent(const std::string &user_agent);
	void referer(const std::string &referer);

	bool operator<(const ConnectionSearchHTTPRequest &rhs) const {
		return _time < rhs._time;
	}

  private:
	uint32_t _http_request_id;
	TimeStamp _time;
	uint8_t _protocol;
	uint32_t _source_ip;
	uint32_t _destination_ip;
	uint32_t _source_port;
	uint32_t _destination_port;
	std::string _type;
	std::string _uri;
	std::string _version;
	std::string _host;
	std::string _user_agent;
	std::string _referer;
};

inline uint32_t ConnectionSearchHTTPRequest::http_request_id() const {
	return _http_request_id;
}

inline uint8_t ConnectionSearchHTTPRequest::protocol() const {
	return _protocol;
}

inline uint32_t ConnectionSearchHTTPRequest::source_ip() const {
	return _source_ip;
}

inline uint32_t ConnectionSearchHTTPRequest::destination_ip() const {
	return _destination_ip;
}

inline uint32_t ConnectionSearchHTTPRequest::source_port() const {
	return _source_port;
}

inline uint32_t ConnectionSearchHTTPRequest::destination_port() const {
	return _destination_port;
}

inline TimeStamp ConnectionSearchHTTPRequest::time() const {
	return _time;
}

inline std::string ConnectionSearchHTTPRequest::type() const {
	return _type;
}

inline std::string ConnectionSearchHTTPRequest::uri() const {
	return _uri;
}

inline std::string ConnectionSearchHTTPRequest::version() const {
	return _version;
}

inline std::string ConnectionSearchHTTPRequest::host() const {
	return _host;
}

inline std::string ConnectionSearchHTTPRequest::user_agent() const {
	return _user_agent;
}

inline std::string ConnectionSearchHTTPRequest::referer() const {
	return _referer;
}

inline void ConnectionSearchHTTPRequest::http_request_id(uint32_t http_request_id) {
	_http_request_id = http_request_id;
}

inline void ConnectionSearchHTTPRequest::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void ConnectionSearchHTTPRequest::source_ip(uint32_t source_ip) {
	_source_ip = source_ip;
}

inline void ConnectionSearchHTTPRequest::destination_ip(uint32_t destination_ip) {
	_destination_ip = destination_ip;
}

inline void ConnectionSearchHTTPRequest::source_port(uint32_t source_port) {
	_source_port = source_port;
}

inline void ConnectionSearchHTTPRequest::destination_port(uint32_t destination_port) {
	_destination_port = destination_port;
}

inline void ConnectionSearchHTTPRequest::time(const TimeStamp &time) {
	_time = time;
}

inline void ConnectionSearchHTTPRequest::type(const std::string &type) {
	_type = type;
}

inline void ConnectionSearchHTTPRequest::uri(const std::string &uri) {
	_uri = uri;
}

inline void ConnectionSearchHTTPRequest::version(const std::string &version) {
	_version = version;
}

inline void ConnectionSearchHTTPRequest::host(const std::string &host) {
	_host = host;
}

inline void ConnectionSearchHTTPRequest::user_agent(const std::string &user_agent) {
	_user_agent = user_agent;
}

inline void ConnectionSearchHTTPRequest::referer(const std::string &referer) {
	_referer = referer;
}

#endif
