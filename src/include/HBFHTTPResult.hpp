#ifndef INFER_INCLUDE_HBFHTTPRESULT_HPP_
#define INFER_INCLUDE_HBFHTTPRESULT_HPP_

#include "HBFResult.hpp"
#include "HTTPResult.hpp"

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

#endif
