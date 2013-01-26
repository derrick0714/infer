#ifndef HBFHTTPRESULT_HPP
#define HBFHTTPRESULT_HPP

#include "HBFResult.hpp"
#include "HTTPResult.hpp"

namespace vn {
namespace arl {
namespace shared {

class HBFHTTPResult {
  public:
	HBFHTTPResult()
		:_hbfResult(),
		 _httpResult()
	{
	}

	HBFHTTPResult(const HBFResult &hbfResult, const HTTPResult &httpResult)
		:_hbfResult(hbfResult),
		 _httpResult(httpResult)
	{
	}

	void assign(const HBFResult &hbfResult, const HTTPResult &httpResult) {
		_hbfResult = hbfResult;
		_httpResult = httpResult;
	}

	const HBFResult & hbf() const {
		return _hbfResult;
	}

	const HTTPResult & http() const {
		return _httpResult;
	}

  private:
	HBFResult _hbfResult;
	HTTPResult _httpResult;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
