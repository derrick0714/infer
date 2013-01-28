#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHDNS_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHDNS_HPP_

class ConnectionSearchDNS {
  public:
	ConnectionSearchDNS()
		:_dns_id(0),
		 _query_time(),
		 _response_time(),
		 _client_ip(0),
		 _server_ip(0),
		 _client_port(0),
		 _server_port(0),
		 _query_name(),
		 _query_type(0)
	{
	}

	uint32_t dns_id() const;
	TimeStamp query_time() const;
	TimeStamp response_time() const;
	uint32_t client_ip() const;
	uint32_t server_ip() const;
	uint16_t client_port() const;
	uint16_t server_port() const;
	std::string query_name() const;
	uint16_t query_type() const;

	void dns_id(uint32_t dns_id);
	void query_time(const TimeStamp &query_time);
	void response_time(const TimeStamp &response_time);
	void client_ip(uint32_t client_ip);
	void server_ip(uint32_t server_ip);
	void client_port(uint16_t client_port);
	void server_port(uint16_t server_port);
	void query_name(const std::string &query_name);
	void query_type(uint16_t query_type);

  private:
	uint32_t _dns_id;
	TimeStamp _query_time;
	TimeStamp _response_time;
	uint32_t _client_ip;
	uint32_t _server_ip;
	uint16_t _client_port;
	uint16_t _server_port;
	std::string _query_name;
	uint16_t _query_type;
};

inline uint32_t ConnectionSearchDNS::dns_id() const {
	return _dns_id;
}

TimeStamp ConnectionSearchDNS::query_time() const {
	return _query_time;
}

TimeStamp ConnectionSearchDNS::response_time() const {
	return _response_time;
}

inline uint32_t ConnectionSearchDNS::client_ip() const {
	return _client_ip;
}

inline uint32_t ConnectionSearchDNS::server_ip() const {
	return _server_ip;
}

inline uint16_t ConnectionSearchDNS::client_port() const {
	return _client_port;
}

inline uint16_t ConnectionSearchDNS::server_port() const {
	return _server_port;
}

inline std::string ConnectionSearchDNS::query_name() const {
	return _query_name;
}

inline uint16_t ConnectionSearchDNS::query_type() const {
	return _query_type;
}

inline void ConnectionSearchDNS::dns_id(uint32_t dns_id) {
	_dns_id = dns_id;
}

inline void ConnectionSearchDNS::query_time(const TimeStamp &query_time) {
	_query_time = query_time;
}

inline void ConnectionSearchDNS::response_time(const TimeStamp &response_time) {
	_response_time = response_time;
}

inline void ConnectionSearchDNS::client_ip(uint32_t client_ip) {
	_client_ip = client_ip;
}

inline void ConnectionSearchDNS::server_ip(uint32_t server_ip) {
	_server_ip = server_ip;
}

inline void ConnectionSearchDNS::client_port(uint16_t client_port) {
	_client_port = client_port;
}

inline void ConnectionSearchDNS::server_port(uint16_t server_port) {
	_server_port = server_port;
}

inline void ConnectionSearchDNS::query_name(const std::string &query_name) {
	_query_name = query_name;
}

inline void ConnectionSearchDNS::query_type(uint16_t query_type) {
	_query_type = query_type;
}

#endif
