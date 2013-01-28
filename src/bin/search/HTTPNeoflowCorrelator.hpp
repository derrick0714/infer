#ifndef INFER_BIN_SEARCH_HTTPNEOFLOWCORRELATOR_HPP_
#define INFER_BIN_SEARCH_HTTPNEOFLOWCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>

#include "SearchNeoflowResult.hpp"
#include "SearchHTTPResult.hpp"

#include "oldHTTP.h"
#include "FlowStats.hpp"
#include "IPv4Network.hpp"
#include "IPv4FlowMatcher.hpp"

template <typename ReaderType, typename WriterType>
class HTTPNeoflowCorrelator {
  public:
	HTTPNeoflowCorrelator()
		:_reader(NULL),
		 _writer(NULL),
		 _set(NULL),
		 _error(false),
		 _errorMsg()
	{
		checkReaderType(typename ReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
	}

	HTTPNeoflowCorrelator(ReaderType *reader,
					  WriterType *writer,
					  const std::set <SearchHTTPResult> *set)
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
			  std::set <SearchHTTPResult> *set)
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
	HTTPNeoflowCorrelator(const HTTPNeoflowCorrelator &);
	const HTTPNeoflowCorrelator & operator=(const HTTPNeoflowCorrelator &);

	void checkReaderType(FlowStats) {}
	void checkWriterType(SearchNeoflowResult) {}

	ReaderType *_reader;
	WriterType *_writer;
	const std::set <SearchHTTPResult> *_set;

	bool _error;
	std::string _errorMsg;
};

template <typename ReaderType, typename WriterType>
int HTTPNeoflowCorrelator<ReaderType, WriterType>::run() {
	FlowStats flowStats;
	IPv4Network network;
	SearchNeoflowResult searchNeoflowResult;

/*
	// prefetch the first HTTP
	ErrorStatus errorStatus;
	if ((errorStatus = _reader->read(http)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		//moreToCorrelate = false;
		return 0;
	}
*/

	std::set <FlowStats> flowSet;
	ErrorStatus errorStatus;

	// read all neoflow from the reader into the queue
	while ((errorStatus = _reader->read(flowStats)) == E_SUCCESS) {
		
		if (flowStats.endTime() >= _set->begin()->time() &&
			flowStats.startTime() <= _set->rbegin()->time())
		{
			flowSet.insert(flowStats);
		}
	}
	std::cerr << "DEBUG: flowSet.size(): " << flowSet.size() << std::endl;

	// for each SearchHTTPResult
	HTTPRequest tmp_request;
	HTTPResponse tmp_response;
	TimeStamp tmp_time;
	for (std::set<SearchHTTPResult>::const_iterator i(_set->begin());
		 i != _set->end() && !flowSet.empty();
		 ++i)
	{
		IPv4FlowMatcher matcher;

		if (i->request().time() != TimeStamp()) {
			tmp_request = i->request();
			network.rawSet(tmp_request.raw_source_ip(), 0xffffffff);
			matcher.addSourceNetwork(network);
			network.rawSet(tmp_request.raw_destination_ip(), 0xffffffff);
			matcher.addDestinationNetwork(network);
			matcher.addSourcePortRange(std::make_pair(tmp_request.source_port(),
													  tmp_request.source_port()));
			matcher.addDestinationPortRange(std::make_pair(
													tmp_request.destination_port(),
													tmp_request.destination_port()));
			tmp_time = tmp_request.time();
		}
		else {
			tmp_response = i->response();
			network.rawSet(tmp_response.raw_source_ip(), 0xffffffff);
			matcher.addSourceNetwork(network);
			network.rawSet(tmp_response.raw_destination_ip(), 0xffffffff);
			matcher.addDestinationNetwork(network);
			matcher.addSourcePortRange(std::make_pair(tmp_response.source_port(),
													  tmp_response.source_port()));
			matcher.addDestinationPortRange(std::make_pair(
													tmp_response.destination_port(),
													tmp_response.destination_port()));
			tmp_time = tmp_response.time();
		}

		// look for the correlated Neoflow.
		std::set<FlowStats>::iterator it(flowSet.begin());
		while(it != flowSet.end()) {
			if (it->endTime() < tmp_time) {
				flowSet.erase(it++);
				continue;
			}
			if (it->startTime() > tmp_time) {
				break;
			}

			if (!matcher.isMatch(*it)) {
				++it;
				continue;
			}

			searchNeoflowResult.index(i->neoflow_index());
			searchNeoflowResult.protocol(it->protocol());
			searchNeoflowResult.rawSourceIP(it->rawSourceIP());
			searchNeoflowResult.rawDestinationIP(it->rawDestinationIP());
			searchNeoflowResult.rawSourcePort(it->rawSourcePort());
			searchNeoflowResult.rawDestinationPort(it->rawDestinationPort());
			searchNeoflowResult.startTime(it->startTime());
			searchNeoflowResult.endTime(it->endTime());
			searchNeoflowResult.numBytes(it->numBytes());
			searchNeoflowResult.numPackets(it->numPackets());

			if (!(_writer->write(searchNeoflowResult))) {
				_error = true;
				_errorMsg.assign("Writer: ");
				_errorMsg.append(_writer->error());
				return 1;
			}

			break;
		}
	}

	return 0;
}

#endif
