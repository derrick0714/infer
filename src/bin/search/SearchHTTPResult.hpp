#ifndef INFER_BIN_SEARCH_SEARCHHTTPRESULT_HPP_
#define INFER_BIN_SEARCH_SEARCHHTTPRESULT_HPP_

#include "HTTPRequest.h"
#include "HTTPResponse.h"

class SearchHTTPResult {
  public:
	SearchHTTPResult()
		:_request(),
		 _response()
	{
	}

	/// \brief Clear this result
	void clear();

	uint32_t neoflow_index() const;
	void neoflow_index(uint32_t neoflow_index);

	HTTPRequest request() const;
	void request(const HTTPRequest &request);

	HTTPResponse response() const;
	void response(const HTTPResponse &response);

	TimeStamp time() const {
		if (_request.time() != TimeStamp()) {
			return _request.time();
		}
		else {
			return _response.time();
		}
	}

	bool operator<(const SearchHTTPResult &rhs) const {
		return time() < rhs.time();
	}

  private:
	uint32_t _neoflow_index;
	HTTPRequest _request;
	HTTPResponse _response;
};

inline void SearchHTTPResult::clear() {
	_request.time(TimeStamp());
	_response.time(TimeStamp());
}

inline uint32_t SearchHTTPResult::neoflow_index() const {
	return _neoflow_index;
}

inline void SearchHTTPResult::neoflow_index(uint32_t neoflow_index) {
	_neoflow_index = neoflow_index;
}

inline HTTPRequest SearchHTTPResult::request() const {
	return _request;
}

inline void SearchHTTPResult::request(const HTTPRequest &request) {
	_request = request;
}

inline HTTPResponse SearchHTTPResult::response() const {
	return _response;
}

inline void SearchHTTPResult::response(const HTTPResponse &response) {
	_response = response;
}

#endif
