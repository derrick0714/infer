#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTP_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTP_HPP_

#include "timeStamp.h"

class ConnectionSearchHTTP {
  public:
	ConnectionSearchHTTP()
		:_http_id(0),
		 _start_time(),
		 _end_time(),
		 _client_ip(0),
		 _server_ip(0),
		 _client_port(0),
		 _server_port(0)
	{
	}

	uint32_t http_id() const;
	TimeStamp start_time() const;
	TimeStamp end_time() const;
	uint8_t protocol() const;
	uint32_t client_ip() const;
	uint32_t server_ip() const;
	uint16_t client_port() const;
	uint16_t server_port() const;

	void http_id(uint32_t http_id);
	void start_time(const TimeStamp &start_time);
	void end_time(const TimeStamp &end_time);
	void protocol(uint8_t protocol);
	void client_ip(uint32_t client_ip);
	void server_ip(uint32_t server_ip);
	void client_port(uint16_t client_port);
	void server_port(uint16_t server_port);

	bool operator<(const ConnectionSearchHTTP &rhs) const {
		return _start_time < rhs._start_time;
	}

  private:
	uint32_t _http_id;
	TimeStamp _start_time;
	TimeStamp _end_time;
	uint8_t _protocol;
	uint32_t _client_ip;
	uint32_t _server_ip;
	uint16_t _client_port;
	uint16_t _server_port;
};

inline uint32_t ConnectionSearchHTTP::http_id() const {
	return _http_id;
}

inline TimeStamp ConnectionSearchHTTP::start_time() const {
	return _start_time;
}

inline TimeStamp ConnectionSearchHTTP::end_time() const {
	return _end_time;
}

inline uint8_t ConnectionSearchHTTP::protocol() const {
	return _protocol;
}

inline uint32_t ConnectionSearchHTTP::client_ip() const {
	return _client_ip;
}

inline uint32_t ConnectionSearchHTTP::server_ip() const {
	return _server_ip;
}

inline uint16_t ConnectionSearchHTTP::client_port() const {
	return _client_port;
}

inline uint16_t ConnectionSearchHTTP::server_port() const {
	return _server_port;
}

inline void ConnectionSearchHTTP::http_id(uint32_t http_id) {
	_http_id = http_id;
}

inline void ConnectionSearchHTTP::start_time(const TimeStamp &start_time) {
	_start_time = start_time;
}

inline void ConnectionSearchHTTP::end_time(const TimeStamp &end_time) {
	_end_time = end_time;
}

inline void ConnectionSearchHTTP::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void ConnectionSearchHTTP::client_ip(uint32_t client_ip) {
	_client_ip = client_ip;
}

inline void ConnectionSearchHTTP::server_ip(uint32_t server_ip) {
	_server_ip = server_ip;
}

inline void ConnectionSearchHTTP::client_port(uint16_t client_port) {
	_client_port = client_port;
}

inline void ConnectionSearchHTTP::server_port(uint16_t server_port) {
	_server_port = server_port;
}

#endif
