#ifndef INFER_BIN_SEARCH_SEARCHDNSRESPONSERESULT_HPP_
#define INFER_BIN_SEARCH_SEARCHDNSRESPONSERESULT_HPP_

#include "DNS.hpp"

class SearchDNSResponseResult {
  public:
	SearchDNSResponseResult()
		:_dns_index(0),
		 _name(),
		 _type(0),
		 _resource_data(),
		 _ttl(0)
	{
	}

	void clear() {
		_dns_index = 0;
		_name.clear();
		_type = 0;
		_resource_data.clear();
		_ttl = 0;
	}

	uint32_t dns_index() const {
		return _dns_index;
	}

	std::string name() const {
		return _name;
	}

	uint16_t type() const {
		return ntohs(_type);
	}

	uint16_t rawType() const {
		return _type;
	}

	std::string resource_data() const {
		return _resource_data;
	}

	int32_t ttl() const {
		return ntohs(_ttl);
	}

	int32_t rawTTL() const {
		return _ttl;
	}


	void dns_index(uint32_t dns_index) {
		_dns_index = dns_index;
	}

	void name(const std::string &name) {
		_name = name;
	}

	void type(uint16_t type) {
		_type = htons(type);
	}

	void rawType(uint16_t rawType) {
		_type = rawType;
	}

	void resource_data(const std::string &resource_data) {
		_resource_data = resource_data;
	}

	void ttl(int32_t ttl) {
		_ttl = htons(ttl);
	}

	void rawTTL(int32_t rawTTL) {
		_ttl = rawTTL;
	}

  private:
	uint32_t _dns_index;
	std::string _name;
	uint16_t _type;
	std::string _resource_data;
	int32_t _ttl;
};

#endif
