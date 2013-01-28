#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHDNSRESPONSE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHDNSRESPONSE_HPP_

#include "DNS.hpp"

class ConnectionSearchDNSResponse {
  public:
	ConnectionSearchDNSResponse()
		:_dns_response_id(0),
		 _name(),
		 _type(0),
		 _resource_data(),
		 _ttl(0)
	{
	}

	uint32_t dns_response_id() const;
	std::string name() const;
	uint16_t type() const;
	uint16_t rawType() const;
	std::string resource_data() const;
	int32_t ttl() const;
	int32_t rawTTL() const;

	void dns_response_id(uint32_t dns_response_id);
	void name(const std::string &name);
	void type(uint16_t type);
	void rawType(uint16_t rawType);
	void resource_data(const std::string &resource_data);
	void ttl(int32_t ttl);
	void rawTTL(int32_t rawTTL);

  private:
	uint32_t _dns_response_id;
	std::string _name;
	uint16_t _type;
	std::string _resource_data;
	int32_t _ttl;
};

inline uint32_t ConnectionSearchDNSResponse::dns_response_id() const {
	return _dns_response_id;
}

inline std::string ConnectionSearchDNSResponse::name() const {
	return _name;
}

inline uint16_t ConnectionSearchDNSResponse::type() const {
	return ntohs(_type);
}

inline uint16_t ConnectionSearchDNSResponse::rawType() const {
	return _type;
}

inline std::string ConnectionSearchDNSResponse::resource_data() const {
	return _resource_data;
}

inline int32_t ConnectionSearchDNSResponse::ttl() const {
	return ntohs(_ttl);
}

inline int32_t ConnectionSearchDNSResponse::rawTTL() const {
	return _ttl;
}

inline void ConnectionSearchDNSResponse::dns_response_id(uint32_t dns_response_id) {
	_dns_response_id = dns_response_id;
}

inline void ConnectionSearchDNSResponse::name(const std::string &name) {
	_name = name;
}

inline void ConnectionSearchDNSResponse::type(uint16_t type) {
	_type = htons(type);
}

inline void ConnectionSearchDNSResponse::rawType(uint16_t rawType) {
	_type = rawType;
}

inline void ConnectionSearchDNSResponse::resource_data(const std::string &resource_data) {
	_resource_data = resource_data;
}

inline void ConnectionSearchDNSResponse::ttl(int32_t ttl) {
	_ttl = htons(ttl);
}

inline void ConnectionSearchDNSResponse::rawTTL(int32_t rawTTL) {
	_ttl = rawTTL;
}

#endif
