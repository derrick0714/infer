#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHNEOFLOWREFERENCE_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHNEOFLOWREFERENCE_HPP_

class ConnectionSearchNeoflowReference {
  public:
	ConnectionSearchNeoflowReference()
		:_connection_id(0),
		 _neoflow_id(0)
	{
	}

	uint32_t connection_id() const;
	uint32_t neoflow_id() const;

	void connection_id(uint32_t connection_id);
	void neoflow_id(uint32_t neoflow_id);

  private:
	uint32_t _connection_id;
	uint32_t _neoflow_id;
};

inline uint32_t ConnectionSearchNeoflowReference::connection_id() const {
	return _connection_id;
}

inline uint32_t ConnectionSearchNeoflowReference::neoflow_id() const {
	return _neoflow_id;
}

inline void ConnectionSearchNeoflowReference::connection_id(uint32_t connection_id) {
	_connection_id = connection_id;
}

inline void ConnectionSearchNeoflowReference::neoflow_id(uint32_t neoflow_id) {
	_neoflow_id = neoflow_id;
}

#endif
