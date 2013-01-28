#ifndef INFER_BIN_SEARCH_NEOFLOWHTTPCORRELATOR_HPP_
#define INFER_BIN_SEARCH_NEOFLOWHTTPCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>

#include "SearchNeoflowResult.hpp"
#include "SearchHTTPResult.hpp"

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "IPv4Network.hpp"
#include "NewIPv4FlowMatcher.hpp"

template <typename RequestReaderType, typename ResponseReaderType, typename WriterType>
class NeoflowHTTPCorrelator {
  public:
	NeoflowHTTPCorrelator()
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

	NeoflowHTTPCorrelator(RequestReaderType *request_reader,
						  ResponseReaderType *response_reader,
						  WriterType *writer,
						  const std::set <SearchNeoflowResult> *set)
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
			  std::set <SearchNeoflowResult> *set)
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
	NeoflowHTTPCorrelator(const NeoflowHTTPCorrelator &);
	const NeoflowHTTPCorrelator & operator=(const NeoflowHTTPCorrelator &);

	void checkRequestReaderType(HTTPRequest) {}
	void checkResponseReaderType(HTTPResponse) {}
	void checkWriterType(SearchHTTPResult) {}

	RequestReaderType *_request_reader;
	ResponseReaderType *_response_reader;
	WriterType *_writer;
	const std::set <SearchNeoflowResult> *_set;

	bool _error;
	std::string _errorMsg;
};

template <typename RequestReaderType, typename ResponseReaderType, typename WriterType>
int NeoflowHTTPCorrelator<RequestReaderType, ResponseReaderType, WriterType>::run() {
	HTTPRequest request;
	HTTPResponse response;
	IPv4Network network;
	SearchHTTPResult searchHTTPResult;
	bool moreRequests(true);
	bool moreResponses(true);

	// prefetch the first HTTPRequest
	ErrorStatus errorStatus;
	if ((errorStatus = _request_reader->read(request)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		//moreRequests = false;
		return 0;
	}

	// prefetch the first HTTPResponse
	if ((errorStatus = _response_reader->read(response)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		//moreRequests = false;
		return 0;
	}

	std::deque <HTTPRequest> request_q;
	std::deque <HTTPResponse> response_q;
	TimeStamp windowSize(1800, 0); // 30 minutes
	// for each SearchNeoflowResult
	for (std::set<SearchNeoflowResult>::const_iterator i(_set->begin());
		 i != _set->end();
		 ++i)
	{
		// fill up the window
		while (!request_q.empty() && request_q.front().time() < i->startTime()) {
			request_q.pop_front();
		}
		while (moreRequests && request.time() <= i -> endTime()) {
			if (request.time() >= i->startTime()) {
				// in range. push it into the deque
				request_q.push_back(request);
			}

			if ((errorStatus = _request_reader->read(request)) != E_SUCCESS) {
				if (errorStatus != E_EOF) {
					_error = true;
					_errorMsg.assign("Reader: error reading");
					return 1;
				}

				// no more request data to read. we're done.
				moreRequests = false;;
			}
		}

		while (!response_q.empty() && response_q.front().time() < i->startTime()) {
			response_q.pop_front();
		}
		while (moreResponses && response.time() <= i -> endTime()) {
			if (response.time() >= i->startTime()) {
				// in range. push it into the deque
				response_q.push_back(response);
			}

			if ((errorStatus = _response_reader->read(response)) != E_SUCCESS) {
				if (errorStatus != E_EOF) {
					_error = true;
					_errorMsg.assign("Reader: error reading");
					return 1;
				}

				// no more response data to read. we're done.
				moreResponses = false;;
			}
		}

		// if there are HTTPs to look at, make flow matcher based on i
		NewIPv4FlowMatcher matcher;
		if (!request_q.empty() || !response_q.empty()) {
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
		}

		// look for the correlated HTTPs.
		for (std::deque<HTTPRequest>::const_iterator it(request_q.begin());
			 it != request_q.end();
			 ++it)
		{
			if (it->time() > i->endTime()) {
				break;
			}
			if (!matcher.isMatch(*it)) {
				continue;
			}

			searchHTTPResult.clear();
			searchHTTPResult.neoflow_index(i->index());
			searchHTTPResult.request(*it);
			
			if (!(_writer->write(searchHTTPResult))) {
				_error = true;
				_errorMsg.assign("Writer: ");
				_errorMsg.append(_writer->error());
				return 1;
			}
		}

		for (std::deque<HTTPResponse>::const_iterator it(response_q.begin());
			 it != response_q.end();
			 ++it)
		{
			if (it->time() > i->endTime()) {
				break;
			}
			if (!matcher.isMatch(*it)) {
				continue;
			}

			searchHTTPResult.clear();
			searchHTTPResult.neoflow_index(i->index());
			searchHTTPResult.response(*it);
			
			if (!(_writer->write(searchHTTPResult))) {
				_error = true;
				_errorMsg.assign("Writer: ");
				_errorMsg.append(_writer->error());
				return 1;
			}
		}
	}

	return 0;
}

#endif
