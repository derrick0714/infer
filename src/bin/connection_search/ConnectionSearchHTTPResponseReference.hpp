#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPRESPONSEREFERENCE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPRESPONSEREFERENCE_HPP_

class ConnectionSearchHTTPResponseReference {
  public:
	ConnectionSearchHTTPResponseReference()
		:_http_id(0),
		 _http_response_id(0)
	{
	}

	uint32_t http_id() const;
	uint32_t http_response_id() const;

	void http_id(uint32_t http_id);
	void http_response_id(uint32_t http_response_id);

  private:
	uint32_t _http_id;
	uint32_t _http_response_id;
};

inline uint32_t ConnectionSearchHTTPResponseReference::http_id() const {
	return _http_id;
}

inline uint32_t ConnectionSearchHTTPResponseReference::http_response_id() const {
	return _http_response_id;
}

inline void ConnectionSearchHTTPResponseReference::http_id(uint32_t http_id) {
	_http_id = http_id;
}

inline void ConnectionSearchHTTPResponseReference::http_response_id(uint32_t http_response_id) {
	_http_response_id = http_response_id;
}

#endif
