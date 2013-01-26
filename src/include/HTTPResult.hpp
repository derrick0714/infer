#ifndef INFER_INCLUDE_HTTPRESULT_HPP_
#define INFER_INCLUDE_HTTPRESULT_HPP_

#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTPResult {
  public:
	HTTPResult()
		:_request(),
		 _response()
	{
	}

	/// \brief Clear this result
	void clear();

	HTTPRequest request() const;
	void request(const HTTPRequest &request);

	HTTPResponse response() const;
	void response(const HTTPResponse &response);

  private:
	HTTPRequest _request;
	HTTPResponse _response;
};

inline void HTTPResult::clear() {
	_request = HTTPRequest();
	_response = HTTPResponse();
}

inline HTTPRequest HTTPResult::request() const {
	return _request;
}

inline void HTTPResult::request(const HTTPRequest &request) {
	_request = request;
}

inline HTTPResponse HTTPResult::response() const {
	return _response;
}

inline void HTTPResult::response(const HTTPResponse &response) {
	_response = response;
}

#endif
