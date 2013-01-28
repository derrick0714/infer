#ifndef INFER_BIN_CONNECTIONSEARCH_HTTPNEOFLOWCORRELATOR_HPP_
#define INFER_BIN_CONNECTIONSEARCH_HTTPNEOFLOWCORRELATOR_HPP_

#include <iostream>
#include <set>
#include <deque>
#include <tr1/unordered_set>

#include "FlowStats.hpp"
#include "IPv4Network.hpp"
#include "IPv4FlowMatcher.hpp"
#include "ErrorStatus.hpp"

#include "ConnectionSearchHTTP.hpp"
#include "ConnectionSearchConnection.hpp"
#include "ConnectionSearchNeoflow.hpp"
#include "ConnectionSearchNeoflowReference.hpp"
#include "ConnectionSearchConnectionHTTPReference.hpp"

template <typename ReaderType,
		  typename ConnectionWriterType,
		  typename FlowWriterType,
		  typename ConnectionFlowRefWriterType,
		  typename ConnectionHTTPRefWriterType>
class HTTPNeoflowCorrelator {
  public:
	HTTPNeoflowCorrelator()
		:_reader(NULL),
		 _connectionWriter(NULL),
		 _flowWriter(NULL),
		 _connFlowRefWriter(NULL),
		 _connHTTPRefWriter(NULL),
		 _set(NULL),
		 _error(false),
		 _errorMsg(),
		 _conn_id(0),
		 _neoflow_id(0)
	{
		checkReaderType(typename ReaderType::value_type());
		checkConnectionWriterType(typename ConnectionWriterType::value_type());
		checkFlowWriterType(typename FlowWriterType::value_type());
		checkConnectionFlowRefWriterType(typename ConnectionFlowRefWriterType::value_type());
		checkConnectionHTTPRefWriterType(typename ConnectionHTTPRefWriterType::value_type());
	}

	HTTPNeoflowCorrelator(ReaderType *reader,
					  ConnectionWriterType *connectionWriter,
					  FlowWriterType *flowWriter,
					  ConnectionFlowRefWriterType *connFlowRefWriter,
					  ConnectionHTTPRefWriterType *connHTTPRefWriter,
					  const std::set <ConnectionSearchHTTP> *set)
		:_reader(reader),
		 _connectionWriter(connectionWriter),
		 _flowWriter(flowWriter),
		 _connFlowRefWriter(connFlowRefWriter),
		 _connHTTPRefWriter(connHTTPRefWriter),
		 _set(set),
		 _error(false),
		 _errorMsg(),
		 _conn_id(0),
		 _neoflow_id(0)
	{
		checkReaderType(typename ReaderType::value_type());
		checkConnectionWriterType(typename ConnectionWriterType::value_type());
		checkFlowWriterType(typename FlowWriterType::value_type());
		checkConnectionFlowRefWriterType(typename ConnectionFlowRefWriterType::value_type());
		checkConnectionHTTPRefWriterType(typename ConnectionHTTPRefWriterType::value_type());
	}

	void init(ReaderType *reader,
			  ConnectionWriterType *connectionWriter,
			  FlowWriterType *flowWriter,
			  ConnectionFlowRefWriterType *connFlowRefWriter,
			  ConnectionHTTPRefWriterType *connHTTPRefWriter,
			  const std::set <ConnectionSearchHTTP> *set)
	{
		_reader = reader;
		_connectionWriter = connectionWriter;
		_flowWriter = flowWriter;
		_connFlowRefWriter = connFlowRefWriter;
		_connHTTPRefWriter = connHTTPRefWriter;
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
	void checkConnectionWriterType(ConnectionSearchConnection) {}
	void checkFlowWriterType(ConnectionSearchNeoflow) {}
	void checkConnectionFlowRefWriterType(ConnectionSearchNeoflowReference) {}
	void checkConnectionHTTPRefWriterType(ConnectionSearchConnectionHTTPReference) {}

	ReaderType *_reader;
	ConnectionWriterType *_connectionWriter;
	FlowWriterType *_flowWriter;
	ConnectionFlowRefWriterType *_connFlowRefWriter;
	ConnectionHTTPRefWriterType *_connHTTPRefWriter;
	const std::set <ConnectionSearchHTTP> *_set;

	bool _error;
	std::string _errorMsg;

	uint32_t _conn_id;
	uint32_t _neoflow_id;
};

template <typename ReaderType,
		  typename ConnectionWriterType,
		  typename FlowWriterType,
		  typename ConnectionFlowRefWriterType,
		  typename ConnectionHTTPRefWriterType>
int HTTPNeoflowCorrelator<ReaderType,
						  ConnectionWriterType,
						  FlowWriterType,
						  ConnectionFlowRefWriterType,
						  ConnectionHTTPRefWriterType>
::run()
{
	FlowStats flowStats;
	IPv4Network network;
	ConnectionSearchNeoflow connectionSearchNeoflow;
	ConnectionSearchConnection *conn_ptr(NULL);
	ConnectionSearchNeoflowReference connFlowRef;
	ConnectionSearchConnectionHTTPReference connHTTPRef;


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
		// FIXME is this correct?
		if (flowStats.endTime() >= _set->begin()->start_time() &&
			flowStats.startTime() <= _set->rbegin()->end_time())
		{
			flowSet.insert(flowStats);
		}
	}
	std::cerr << "DEBUG: flowSet.size(): " << flowSet.size() << std::endl;

	// for each ConnectionSearchNeoflow
	std::tr1::unordered_set<uint32_t> neoflow_id_set;
	for (std::set<ConnectionSearchHTTP>::const_iterator i(_set->begin());
		 i != _set->end() && !flowSet.empty();
		 ++i)
	{
		IPv4FlowMatcher matcher;

		network.set(i->client_ip(), 0xffffffff);
		matcher.addSourceNetwork(network);
		network.set(i->server_ip(), 0xffffffff);
		matcher.addDestinationNetwork(network);
		matcher.addSourcePortRange(std::make_pair(i->client_port(),
												  i->client_port()));
		matcher.addDestinationPortRange(std::make_pair(
												i->server_port(),
												i->server_port()));

		IPv4FlowMatcher reverseMatcher(matcher.reverse());

		// look for the correlated Neoflow.
		std::set<FlowStats>::iterator it(flowSet.begin());
		while(it != flowSet.end()) {
			if (it->endTime() < i->start_time()) {
				flowSet.erase(it++);
				continue;
			}
			if (it->startTime() > i->end_time()) {
				break;
			}

			if (!matcher.isMatch(*it) && !reverseMatcher.isMatch(*it)) {
				++it;
				continue;
			}

			// check if this connection is already in the map
			if (conn_ptr == NULL) {
				// formulate the id
				if (it->rawSourceIP() < it->rawDestinationIP()) {
					conn_ptr = new ConnectionSearchConnection(_conn_id,
													it->protocol(),
													it->rawSourceIP(),
													it->rawDestinationIP(),
													it->rawSourcePort(),
													it->rawDestinationPort());
				}
				else {
					conn_ptr = new ConnectionSearchConnection(_conn_id,
													it->protocol(),
													it->rawDestinationIP(),
													it->rawSourceIP(),
													it->rawDestinationPort(),
													it->rawSourcePort());
				}

				++_conn_id;
			}

			neoflow_id_set.insert(_neoflow_id);
			if (it->startTime() < conn_ptr->start_time()) {
				conn_ptr->start_time(it->startTime());
			}
			if (it->endTime() > conn_ptr->end_time()) {
				conn_ptr->end_time(it->endTime());
			}

			connectionSearchNeoflow.neoflow_id(_neoflow_id);
			connectionSearchNeoflow.protocol(it->protocol());
			connectionSearchNeoflow.raw_source_ip(it->rawSourceIP());
			connectionSearchNeoflow.raw_destination_ip(it->rawDestinationIP());
			connectionSearchNeoflow.raw_source_port(it->rawSourcePort());
			connectionSearchNeoflow.raw_destination_port(it->rawDestinationPort());
			connectionSearchNeoflow.start_time(it->startTime());
			connectionSearchNeoflow.end_time(it->endTime());
			connectionSearchNeoflow.byte_count(it->numBytes());
			connectionSearchNeoflow.packet_count(it->numPackets());

			if (!(_flowWriter->write(connectionSearchNeoflow))) {
				_error = true;
				_errorMsg.assign("Flow Writer: ");
				_errorMsg.append(_flowWriter->error());
				return 1;
			}

			++_neoflow_id;
			++it;
		}

		if (conn_ptr == NULL) {
			continue;
		}

		// write the connection
		if (!(_connectionWriter->write(*conn_ptr))) {
			_error = true;
			_errorMsg.assign("Connection Writer: ");
			_errorMsg.append(_connectionWriter->error());
			return 1;
		}

		// write the cross references
		connFlowRef.connection_id(conn_ptr->connection_id());
		for (std::tr1::unordered_set<uint32_t>::iterator id(neoflow_id_set.begin());
			 id != neoflow_id_set.end();
			 ++id)
		{
			connFlowRef.neoflow_id(*id);

			if (!(_connFlowRefWriter->write(connFlowRef))) {
				_error = true;
				_errorMsg.assign("Connection Flow Reference Writer: ");
				_errorMsg.append(_connFlowRefWriter->error());
				return 1;
			}
		}
		neoflow_id_set.clear();

		connHTTPRef.connection_id(conn_ptr->connection_id());
		connHTTPRef.http_id(i->http_id());
		if (!(_connHTTPRefWriter->write(connHTTPRef))) {
			_error = true;
			_errorMsg.assign("Connection HTTP Reference Writer: ");
			_errorMsg.append(_connHTTPRefWriter->error());
			return 1;
		}

		delete conn_ptr;
		conn_ptr = NULL;
	}

	return 0;
}

#endif
