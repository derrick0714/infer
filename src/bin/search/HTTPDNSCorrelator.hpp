#ifndef INFER_INCLUDE_HTTPDNSCORRELATOR_HPP_
#define INFER_INCLUDE_HTTPDNSCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>

#include "oldHTTP.h"
#include "HBFResult.hpp"
#include "HTTPResult.hpp"
#include "SearchDNSResult.hpp"
#include "SearchDNSResponseResult.hpp"
#include "IPv4Network.hpp"
#include "IPv4FlowMatcher.hpp"

template <typename ReaderType, typename WriterType, typename ResponseWriterType>
class HTTPDNSCorrelator {
  public:
	HTTPDNSCorrelator()
		:_reader(NULL),
		 _writer(NULL),
		 _responseWriter(NULL),
		 _set(NULL),
		 _dnsServers(NULL),
		 _dns_index(0),
		 _error(false),
		 _errorMsg()
	{
		checkReaderType(typename ReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
		checkResponseWriterType(typename ResponseWriterType::value_type());
	}

	HTTPDNSCorrelator(ReaderType *reader,
					  WriterType *writer,
					  ResponseWriterType *responseWriter,
					  const std::set <SearchHTTPResult> *set,
					  const std::tr1::unordered_set<uint32_t> *dnsServers)
		:_reader(reader),
		 _writer(writer),
		 _responseWriter(responseWriter),
		 _set(set),
		 _dnsServers(dnsServers),
		 _dns_index(0),
		 _error(false),
		 _errorMsg()
	{
		checkReaderType(typename ReaderType::value_type());
		checkWriterType(typename WriterType::value_type());
		checkResponseWriterType(typename ResponseWriterType::value_type());
	}

	void init(ReaderType *reader,
			  WriterType *writer,
			  ResponseWriterType *responseWriter,
			  const std::set <SearchHTTPResult> *set,
			  const std::tr1::unordered_set<uint32_t> *dnsServers)
	{
		_reader = reader;
		_writer = writer;
		_responseWriter = responseWriter;
		_set = set;
		_dnsServers = dnsServers;
	}

	int run();

	std::string error() const {
		return _errorMsg;
	}

  private:
	HTTPDNSCorrelator(const HTTPDNSCorrelator &);
	const HTTPDNSCorrelator & operator=(const HTTPDNSCorrelator &);

	void checkReaderType(DNS) {}
	void checkWriterType(SearchDNSResult) {}
	void checkResponseWriterType(SearchDNSResponseResult) {}

	ReaderType *_reader;
	WriterType *_writer;
	ResponseWriterType *_responseWriter;
	const std::set <SearchHTTPResult> *_set;
	const std::tr1::unordered_set<uint32_t> *_dnsServers;
	uint32_t _dns_index;

	bool _error;
	std::string _errorMsg;
};

template <typename ReaderType, typename WriterType, typename ResponseWriterType>
int HTTPDNSCorrelator<ReaderType, WriterType, ResponseWriterType>::run() {
	IPv4Network network;
	DNS dns;
	SearchDNSResult dnsResult;
	SearchDNSResponseResult dnsResponseResult;
	bool moreToCorrelate(true);

	// prefetch the first DNS
	ErrorStatus errorStatus;
	if ((errorStatus = _reader->read(dns)) != E_SUCCESS) {
		if (errorStatus != E_EOF) {
			_error = true;
			_errorMsg.assign("Reader: error reading ");
			return 1;
		}
		// no more http data to read. we're done.
		moreToCorrelate = false;
	}

	std::deque <DNS> _q;
	TimeStamp windowSize(300, 0); // 5 minutes
	uint32_t tmpRawSourceIP, tmpRawDestinationIP;
	// for each SearchNeoflowResult
	for (std::set<SearchHTTPResult>::const_iterator i(_set->begin());
		 i != _set->end();
		 ++i)
	{
		// fill up the 5 minute window
		while (!_q.empty() && _q.front().responseTime() < i->time() - windowSize) {
			_q.pop_front();
		}
		while (moreToCorrelate && dns.responseTime() <= i->time()) {
			if (dns.responseTime() >= i->time() - windowSize) {
				// in range. push it into the deque
				_q.push_back(dns);
			}

			if ((errorStatus = _reader->read(dns)) != E_SUCCESS) {
				if (errorStatus != E_EOF) {
					_error = true;
					_errorMsg.assign("Reader: error reading");
					return 1;
				}

				// no more http data to read. we're done.
				moreToCorrelate = false;
			}
		}

		// if there are HTTPs to look at, make flow matcher based on i
		// FIXME this should make a matcher that matches DNS flows
		// where the client is the INTERNAL address of the flow
		// for now we just match either, as this will probably suffice
		/*
		IPv4FlowMatcher matcher, reverseMatcher;
		if (!_q.empty()) {
			network.rawSet(i->rawSourceIP(), 0xffffffff);
			matcher.addDestinationNetwork(network);

			network.rawSet(i->rawDestinationIP(), 0xffffffff);
			reverseMatcher.addDestinationNetwork(network);
		}
		*/
		if (i->request().time() != TimeStamp()) {
			tmpRawSourceIP = i->request().raw_source_ip();
			tmpRawDestinationIP = i->request().raw_destination_ip();
		}
		else {
			tmpRawSourceIP = i->response().raw_source_ip();
			tmpRawDestinationIP = i->response().raw_destination_ip();
		}

		// look for the correlated DNSs.
		bool foundMatch(false);
		DNS *serverDNS(NULL);
		for (std::deque <DNS>::const_reverse_iterator it(_q.rbegin());
			 it != _q.rend();
			 ++it)
		{
			if (tmpRawSourceIP != it->rawClientIP() &&
				tmpRawDestinationIP != it->rawClientIP() &&
				(_dnsServers == NULL || _dnsServers->find(it->rawClientIP()) == _dnsServers->end()))
			{
				continue;
			}

			const std::vector<DNS::DNSResponse*> &responses(it->responses());
			foundMatch = false;
			for (std::vector<DNS::DNSResponse*>::const_iterator r(responses.begin());
				 r != responses.end();
				 ++r)
			{
				if ((*r)->type() == 1) {
					if ((tmpRawSourceIP == it->rawClientIP()) &&
						(tmpRawDestinationIP == *reinterpret_cast<const uint32_t *>((*r)->resourceData().data())))
					{
						foundMatch = true;
						break;
					}
					else if ((tmpRawDestinationIP == it->rawClientIP()) &&
							 (tmpRawSourceIP == *reinterpret_cast<const uint32_t *>((*r)->resourceData().data())))
					{
						foundMatch = true;
						break;
					}
					else if (serverDNS == NULL &&
							 (tmpRawSourceIP == *reinterpret_cast<const uint32_t *>((*r)->resourceData().data()) ||
							  tmpRawDestinationIP == *reinterpret_cast<const uint32_t *>((*r)->resourceData().data())))
					{
						serverDNS = new DNS(*it);
					}
				}
			}

			if (!foundMatch) {
				continue;
			}

			// create SearchDNSResult from DNS and write it.
			dnsResult.index(_dns_index);
			dnsResult.neoflow_index(i->neoflow_index());
			dnsResult.queryTime(it->queryTime());
			dnsResult.responseTime(it->responseTime());
			dnsResult.raw_client_ip(it->rawClientIP());
			dnsResult.raw_server_ip(it->rawServerIP());
			dnsResult.queryName(it->queryName());
			dnsResult.rawQueryType(it->rawQueryType());

			if (!(_writer->write(dnsResult))) {
				_error = true;
				_errorMsg.assign("Writer: ");
				_errorMsg.append(_writer->error());
				return 1;
			}

			// create a SearchDNSResponseResult for each result and write them
			for (std::vector<DNS::DNSResponse*>::const_iterator r(responses.begin());
				 r != responses.end();
				 ++r)
			{
				dnsResponseResult.dns_index(_dns_index);
				dnsResponseResult.name((*r)->name());
				dnsResponseResult.rawType((*r)->rawType());
				dnsResponseResult.resource_data((*r)->resourceData());
				dnsResponseResult.rawTTL((*r)->rawTTL());

				if (!(_responseWriter->write(dnsResponseResult))) {
					_error = true;
					_errorMsg.assign("Writer: ");
					_errorMsg.append(_responseWriter->error());
					return 1;
				}
			}

			++_dns_index;

			break;
		}
		if (!foundMatch && serverDNS != NULL) {
			// create SearchDNSResult from serverDNS and write it.
			dnsResult.index(_dns_index);
			dnsResult.neoflow_index(i->neoflow_index());
			dnsResult.queryTime(serverDNS->queryTime());
			dnsResult.responseTime(serverDNS->responseTime());
			dnsResult.queryName(serverDNS->queryName());
			dnsResult.rawQueryType(serverDNS->rawQueryType());

			if (!(_writer->write(dnsResult))) {
				_error = true;
				_errorMsg.assign("Writer: ");
				_errorMsg.append(_writer->error());
				return 1;
			}

			// create a SearchDNSResponseResult for each result and write them
			const std::vector<DNS::DNSResponse*> &responses(serverDNS->responses());
			for (std::vector<DNS::DNSResponse*>::const_iterator r(responses.begin());
				 r != responses.end();
				 ++r)
			{
				dnsResponseResult.dns_index(_dns_index);
				dnsResponseResult.name((*r)->name());
				dnsResponseResult.rawType((*r)->rawType());
				dnsResponseResult.resource_data((*r)->resourceData());
				dnsResponseResult.rawTTL((*r)->rawTTL());

				if (!(_responseWriter->write(dnsResponseResult))) {
					_error = true;
					_errorMsg.assign("Writer: ");
					_errorMsg.append(_responseWriter->error());
					return 1;
				}
			}

			++_dns_index;

			delete serverDNS;
			serverDNS = NULL;
		}
	}

	return 0;
}

#endif
