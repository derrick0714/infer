#ifndef INFER_BIN_SEARCH_SEARCHDNSRESULT_HPP_
#define INFER_BIN_SEARCH_SEARCHDNSRESULT_HPP_

#include "DNS.hpp"

class SearchDNSResult {
  public:
	SearchDNSResult()
		:_index(0),
		 _neoflow_index(0),
		 _client_ip(0),
		 _server_ip(0),
		 _queryTime(),
		 _responseTime(),
		 _queryName(),
		 _queryType(0)
	{
	}

	void clear() {
		_index = 0;
		_neoflow_index = 0;
		_client_ip = 0;
		_server_ip = 0;
		_queryTime.set(0, 0);
		_responseTime.set(0, 0);
		_queryName.clear();
		_queryType = 0;
	}

	uint32_t index() const {
		return _index;
	}

	uint32_t neoflow_index() const {
		return _neoflow_index;
	}

	uint32_t client_ip() const {
		return ntohl(_client_ip);
	}

	uint32_t raw_client_ip() const {
		return _client_ip;
	}

	uint32_t server_ip() const {
		return ntohl(_server_ip);
	}

	uint32_t raw_server_ip() const {
		return _server_ip;
	}

	TimeStamp queryTime() const {
		return _queryTime;
	}

	TimeStamp responseTime() const {
		return _responseTime;
	}

	std::string queryName() const {
		return _queryName;
	}

	uint16_t queryType() const {
		return ntohs(_queryType);
	}

	uint16_t rawQueryType() const {
		return _queryType;
	}

	
	void index(uint32_t index) {
		_index = index;
	}

	void neoflow_index(uint32_t neoflow_index) {
		_neoflow_index = neoflow_index;
	}

	void client_ip(uint32_t client_ip) {
		_client_ip = htonl(client_ip);
	}

	void raw_client_ip(uint32_t raw_client_ip) {
		_client_ip = raw_client_ip;
	}

	void server_ip(uint32_t server_ip) {
		_server_ip = htonl(server_ip);
	}

	void raw_server_ip(uint32_t raw_server_ip) {
		_server_ip = raw_server_ip;
	}

	void queryTime(const TimeStamp &queryTime) {
		_queryTime = queryTime;
	}

	void responseTime(const TimeStamp &responseTime) {
		_responseTime = responseTime;
	}

	void queryName(const std::string &queryName) {
		_queryName = queryName;
	}

	void queryType(uint16_t queryType) {
		_queryType = htons(queryType);
	}

	void rawQueryType(uint16_t rawQueryType) {
		_queryType = rawQueryType;
	}

  private:
	uint32_t _index;
	uint32_t _neoflow_index;
	uint32_t _client_ip;
	uint32_t _server_ip;
	TimeStamp _queryTime;
	TimeStamp _responseTime;
	std::string _queryName;
	uint16_t _queryType;
};

#endif
