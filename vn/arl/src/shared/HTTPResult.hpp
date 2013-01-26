#ifndef HTTPRESULT_HPP
#define HTTPRESULT_HPP

#include "HTTP.h"

namespace vn {
namespace arl {
namespace shared {

class HTTPResult {
  public:
	HTTPResult()
		:_request(),
		 _response()
	{
	}

	/// \brief Clear this result
	void clear();

	HTTP request() const;
	void request(const HTTP &request);

	HTTP response() const;
	void response(const HTTP &response);

  private:
	HTTP _request;
	HTTP _response;
};

inline void HTTPResult::clear() {
	_request.clear();
	_response.clear();
}

inline HTTP HTTPResult::request() const {
	return _request;
}

inline void HTTPResult::request(const HTTP &request) {
	_request = request;
}

inline HTTP HTTPResult::response() const {
	return _response;
}

inline void HTTPResult::response(const HTTP &response) {
	_response = response;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
