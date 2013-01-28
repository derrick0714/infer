#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHCONNECTIONHTTPREFERENCE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHCONNECTIONHTTPREFERENCE_HPP_

class ConnectionSearchConnectionHTTPReference {
  public:
	ConnectionSearchConnectionHTTPReference()
		:_connection_id(0),
		 _http_id(0)
	{
	}

	uint32_t connection_id() const;
	uint32_t http_id() const;

	void connection_id(uint32_t connection_id);
	void http_id(uint32_t http_id);

  private:
	uint32_t _connection_id;
	uint32_t _http_id;
};

inline uint32_t ConnectionSearchConnectionHTTPReference::connection_id() const {
	return _connection_id;
}

inline uint32_t ConnectionSearchConnectionHTTPReference::http_id() const {
	return _http_id;
}

inline void ConnectionSearchConnectionHTTPReference::connection_id(uint32_t connection_id) {
	_connection_id = connection_id;
}

inline void ConnectionSearchConnectionHTTPReference::http_id(uint32_t http_id) {
	_http_id = http_id;
}

#endif
