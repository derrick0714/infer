#ifndef INFER_INCLUDE_HBFHTTPCORRELATOR_HPP_
#define INFER_INCLUDE_HBFHTTPCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>

#include "oldHTTP.h"
#include "HBFResult.hpp"
#include "HTTPResult.hpp"
#include "HBFHTTPResult.hpp"
#include "IPv4Network.hpp"
#include "NewIPv4FlowMatcher.hpp"

template <typename RequestReaderType, typename ResponseReaderType, typename WriterType>
class HBFHTTPCorrelator {
  public:
	HBFHTTPCorrelator()
		:_request_reader(NULL),
		 _response_reader(NULL),
		 _writer(NULL),
		 _set(NULL),
		 _error(false),
		 _errorMsg()
	{
		checkRequestReaderType(typename RequestReaderType::value_type());
		checkResponseReaderType(typename ResponseReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
	}

	HBFHTTPCorrelator(RequestReaderType *request_reader,
					  ResponseReaderType *response_reader,
					  WriterType *writer,
					  const std::set <HBFResult> *set)
		:_request_reader(request_reader),
		 _response_reader(response_reader),
		 _writer(writer),
		 _set(set),
		 _error(false),
		 _errorMsg()
	{
		checkRequestReaderType(typename RequestReaderType::value_type());
		checkResponseReaderType(typename ResponseReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
	}

	void init(RequestReaderType *request_reader,
			  ResponseReaderType *response_reader,
			  WriterType *writer,
			  std::set <HBFResult> *set)
	{
		_request_reader = request_reader;
		_response_reader = response_reader;
		_writer = writer;
		_set = set;
	}

	int run();

	std::string error() const {
		return _errorMsg;
	}

  private:
	HBFHTTPCorrelator(const HBFHTTPCorrelator &);
	const HBFHTTPCorrelator & operator=(const HBFHTTPCorrelator &);

	void checkRequestReaderType(HTTPRequest) {}
	void checkResponseReaderType(HTTPResponse) {}
	void checkWriterType(HBFHTTPResult) {}

	RequestReaderType *_request_reader;
	ResponseReaderType *_response_reader;
	WriterType *_writer;
	const std::set <HBFResult> *_set;

	bool _error;
	std::string _errorMsg;
};

template <typename RequestReaderType, typename ResponseReaderType, typename WriterType>
int HBFHTTPCorrelator<RequestReaderType, ResponseReaderType, WriterType>::run() {
	HTTPRequest http_request;
	HTTPResponse http_response;
	IPv4Network network;
	NewIPv4FlowMatcher matcher, reverseMatcher;
	bool foundRequest(false), foundResponse(false);
	HTTPResult httpResult;
	HBFHTTPResult hbfhttpResult;
	bool moreRequests(true);
	bool moreResponses(true);

	// prefetch the first HTTP
	ErrorStatus errorStatus;
	if ((errorStatus = _request_reader->read(http_request)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		moreRequests = false;
	}

	if ((errorStatus = _response_reader->read(http_response)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		moreResponses = false;
	}

	std::deque <HTTPRequest> request_q;
	std::deque <HTTPResponse> response_q;
	TimeStamp windowSize(300, 0); // 5 minutes
	// for each HBFResult
	for (std::set<HBFResult>::const_iterator i(_set->begin());
		 i != _set->end();
		 ++i)
	{
		// fill up the 5 minute window for requests
		while (!request_q.empty() && request_q.front().time() < i->startTime() - windowSize) {
			request_q.pop_front();
		}
		while (moreRequests && http_request.time() <= i -> startTime()) {
			if (http_request.time() >= i->startTime() - windowSize) {
				// in range. push it into the deque
				request_q.push_back(http_request);
			}

			if ((errorStatus = _request_reader->read(http_request)) != E_SUCCESS) {
				if (errorStatus != E_EOF) {
					_error = true;
					_errorMsg.assign("Reader: error reading");
					return 1;
				}

				// no more http_request data to read. we're done.
				moreRequests = false;
			}
		}

		// fill up the 5 minute window for responses
		while (!response_q.empty() && response_q.front().time() < i->startTime() - windowSize) {
			response_q.pop_front();
		}
		while (moreResponses && http_response.time() <= i -> startTime()) {
			if (http_response.time() >= i->startTime() - windowSize) {
				// in range. push it into the deque
				response_q.push_back(http_response);
			}

			if ((errorStatus = _response_reader->read(http_response)) != E_SUCCESS) {
				if (errorStatus != E_EOF) {
					_error = true;
					_errorMsg.assign("Reader: error reading");
					return 1;
				}

				// no more http_response data to read. we're done.
				moreResponses = false;;
			}
		}

		// if there are HTTPs to look at, make flow matcher based on i
		if (!request_q.empty() && !response_q.empty()) {
			matcher.addProtocolRange(std::make_pair(i->protocol(),
													i->protocol()));
			network.rawSet(i->rawSourceIP(), 0xffffffff);
			matcher.addSourceNetwork(network);
			network.rawSet(i->rawDestinationIP(), 0xffffffff);
			matcher.addDestinationNetwork(network);
			matcher.addSourcePortRange(std::make_pair(i->sourcePort(),
													  i->sourcePort()));
			matcher.addDestinationPortRange(std::make_pair(
													i->destinationPort(),
													i->destinationPort()));

			reverseMatcher.addProtocolRange(std::make_pair(i->protocol(),
														   i->protocol()));
			network.rawSet(i->rawDestinationIP(), 0xffffffff);
			reverseMatcher.addSourceNetwork(network);
			network.rawSet(i->rawSourceIP(), 0xffffffff);
			reverseMatcher.addDestinationNetwork(network);
			reverseMatcher.addSourcePortRange(std::make_pair(
													i->destinationPort(),
													i->destinationPort()));
			reverseMatcher.addDestinationPortRange(std::make_pair(
													i->sourcePort(),
													i->sourcePort()));
		}

		// look for the correlated HTTP request.
		foundRequest = foundResponse = false;
		for (std::deque <HTTPRequest>::reverse_iterator it(request_q.rbegin());
			 it != request_q.rend();
			 ++it)
		{
			if (matcher.isMatch(*it) || reverseMatcher.isMatch(*it)) {
				http_request = *it;
				foundRequest = true;
				break;
			}
		}

		for (std::deque <HTTPResponse>::reverse_iterator it(response_q.rbegin());
			 it != response_q.rend();
			 ++it)
		{
			if (matcher.isMatch(*it) || reverseMatcher.isMatch(*it)) {
				http_response = *it;
				foundResponse = true;
				break;
			}
		}

		// prepare the result
		httpResult.clear();
		if (foundRequest) {
			// we found a matching Request.
			httpResult.request(http_request);
		}
		if (foundResponse) {
			// we found a matching Response.
			httpResult.response(http_response);
		}
		hbfhttpResult.assign(*i, httpResult);
		if (!(_writer->write(hbfhttpResult))) {
			_error = true;
			_errorMsg.assign("Writer: ");
			_errorMsg.append(_writer->error());
			return 1;
		}
	}

	return 0;
}

#endif
