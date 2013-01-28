#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPREQUESTREFERENCE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHHTTPREQUESTREFERENCE_HPP_

class ConnectionSearchHTTPRequestReference {
  public:
	ConnectionSearchHTTPRequestReference()
		:_http_id(0),
		 _http_request_id(0)
	{
	}

	uint32_t http_id() const;
	uint32_t http_request_id() const;

	void http_id(uint32_t http_id);
	void http_request_id(uint32_t http_request_id);

  private:
	uint32_t _http_id;
	uint32_t _http_request_id;
};

inline uint32_t ConnectionSearchHTTPRequestReference::http_id() const {
	return _http_id;
}

inline uint32_t ConnectionSearchHTTPRequestReference::http_request_id() const {
	return _http_request_id;
}

inline void ConnectionSearchHTTPRequestReference::http_id(uint32_t http_id) {
	_http_id = http_id;
}

inline void ConnectionSearchHTTPRequestReference::http_request_id(uint32_t http_request_id) {
	_http_request_id = http_request_id;
}

#endif
