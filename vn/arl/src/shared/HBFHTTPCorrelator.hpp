#ifndef HBFHTTPCORRELATOR_HPP
#define HBFHTTPCORRELATOR_HPP

#include <iostream>
#include <set>
#include <deque>

#include "HTTP.h"
#include "HBFResult.hpp"
#include "HTTPResult.hpp"
#include "HBFHTTPResult.hpp"
#include "IPv4Network.hpp"
#include "IPv4FlowMatcher.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename ReaderType, typename WriterType>
class HBFHTTPCorrelator {
  public:
	HBFHTTPCorrelator()
		:_reader(NULL),
		 _writer(NULL),
		 _set(NULL),
		 _error(false),
		 _errorMsg()
	{
		checkReaderType(typename ReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
	}

	HBFHTTPCorrelator(ReaderType *reader,
					  WriterType *writer,
					  const std::set <HBFResult> *set)
		:_reader(reader),
		 _writer(writer),
		 _set(set),
		 _error(false),
		 _errorMsg()
	{
		checkReaderType(typename ReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
	}

	void init(ReaderType *reader,
			  WriterType *writer,
			  std::set <HBFResult> *set)
	{
		_reader = reader;
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

	void checkReaderType(HTTP) {}
	void checkWriterType(HBFHTTPResult) {}

	ReaderType *_reader;
	WriterType *_writer;
	const std::set <HBFResult> *_set;

	bool _error;
	std::string _errorMsg;
};

template <typename ReaderType, typename WriterType>
int HBFHTTPCorrelator<ReaderType, WriterType>::run() {
	HTTP http, matchingRequest, matchingResponse;
	IPv4Network network;
	IPv4FlowMatcher matcher, reverseMatcher;
	bool foundRequest(false), foundResponse(false);
	HTTPResult httpResult;
	HBFHTTPResult hbfhttpResult;
	bool moreToCorrelate(true);

	// prefetch the first HTTP
	if (!(_reader->read(http))) {
		if (!(*_reader)) {
			_error = true;
			_errorMsg.assign("Reader: ");
			_errorMsg.append(_reader->error());
			return 1;
		}
		// no more http data to read. we're done.
		moreToCorrelate = false;
	}

	std::deque <HTTP> _q;
	TimeStamp windowSize(300, 0); // 5 minutes
	// for each HBFResult
	for (std::set<HBFResult>::const_iterator i(_set->begin());
		 i != _set->end();
		 ++i)
	{
		// fill up the 5 minute window
		while (!_q.empty() && _q.front().time() < i->startTime() - windowSize) {
			_q.pop_front();
		}
		while (moreToCorrelate && http.time() <= i -> startTime()) {
			if (http.time() >= i->startTime() - windowSize) {
				// in range. push it into the deque
				_q.push_back(http);
			}

			if (!(_reader->read(http))) {
				if (!(*_reader)) {
					_error = true;
					_errorMsg.assign("Reader: ");
					_errorMsg.append(_reader->error());
					return 1;
				}

				// no more http data to read. we're done.
				moreToCorrelate = false;;
			}
		}

		// if there are HTTPs to look at, make flow matcher based on i
		if (!_q.empty()) {
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

		// look for the correlated HTTPs.
		foundRequest = foundResponse = false;
		for (std::deque <HTTP>::reverse_iterator it(_q.rbegin());
			 it != _q.rend();
			 ++it)
		{
			if (matcher.isMatch(*it) || reverseMatcher.isMatch(*it)) {
				if (!foundRequest && it->type() == 'q') {
					matchingRequest = *it;
					foundRequest = true;
				}
				if (!foundResponse && it->type() == 's') {
					matchingResponse = *it;
					foundResponse = true;
				}
			}
			if (foundRequest && foundResponse) {
				break;
			}
		}

		// prepare the result
		httpResult.clear();
		if (foundRequest) {
			// we found a matching Request.
			httpResult.request(matchingRequest);
		}
		if (foundResponse) {
			// we found a matching Response.
			httpResult.response(matchingResponse);
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

} // namespace shared
} // namespace arl
} // namespace vn

#endif
