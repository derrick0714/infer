#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHNEOFLOW_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHNEOFLOW_HPP_

#include <sys/types.h>
#include <netinet/in.h>

#include "timeStamp.h"

class ConnectionSearchNeoflow {
  public:
	ConnectionSearchNeoflow();

	uint32_t neoflow_id() const;
	uint8_t protocol() const;
	uint32_t source_ip() const;
	uint32_t raw_source_ip() const;
	uint32_t destination_ip() const;
	uint32_t raw_destination_ip() const;
	uint16_t source_port() const;
	uint16_t raw_source_port() const;
	uint16_t destination_port() const;
	uint16_t raw_destination_port() const;
	TimeStamp start_time() const;
	TimeStamp end_time() const;
	uint32_t byte_count() const;
	uint32_t packet_count() const;

	void neoflow_id(uint32_t neoflow_id);
	void protocol(uint8_t protocol);
	void source_ip(uint32_t source_ip);
	void raw_source_ip(uint32_t source_ip);
	void destination_ip(uint32_t destination_ip);
	void raw_destination_ip(uint32_t destination_ip);
	void source_port(uint16_t source_port);
	void raw_source_port(uint16_t source_port);
	void destination_port(uint16_t destination_port);
	void raw_destination_port(uint16_t destination_port);
	void start_time(const TimeStamp &start_time);
	void end_time(const TimeStamp &end_time);
	void byte_count(uint32_t byte_count);
	void packet_count(uint32_t packet_count);

	bool operator<(const ConnectionSearchNeoflow &rhs) const {
		return _start_time < rhs._start_time;
	}

  private:
	uint32_t _neoflow_id;
	uint8_t _protocol;
	uint32_t _source_ip;
	uint32_t _destination_ip;
	uint16_t _source_port;
	uint16_t _destination_port;
	TimeStamp _start_time;
	TimeStamp _end_time;
	uint32_t _byte_count;
	uint32_t _packet_count;
};

inline ConnectionSearchNeoflow::ConnectionSearchNeoflow()
	:_neoflow_id(0),
	 _protocol(0),
	 _source_ip(0),
	 _destination_ip(0),
	 _source_port(0),
	 _destination_port(0),
	 _start_time(),
	 _end_time(),
	 _byte_count(0),
	 _packet_count(0)
{
}

inline uint32_t ConnectionSearchNeoflow::neoflow_id() const {
	return _neoflow_id;
}

inline uint8_t ConnectionSearchNeoflow::protocol() const {
	return _protocol;
}

inline uint32_t ConnectionSearchNeoflow::source_ip() const {
	return ntohl(raw_source_ip());
}

inline uint32_t ConnectionSearchNeoflow::destination_ip() const {
	return ntohl(raw_destination_ip());
}

inline uint16_t ConnectionSearchNeoflow::source_port() const {
	return ntohs(raw_source_port());
}

inline uint16_t ConnectionSearchNeoflow::destination_port() const {
	return ntohs(raw_destination_port());
}

inline uint32_t ConnectionSearchNeoflow::raw_source_ip() const {
	return _source_ip;
}

inline uint32_t ConnectionSearchNeoflow::raw_destination_ip() const {
	return _destination_ip;
}

inline uint16_t ConnectionSearchNeoflow::raw_source_port() const {
	return _source_port;
}

inline uint16_t ConnectionSearchNeoflow::raw_destination_port() const {
	return _destination_port;
}

inline TimeStamp ConnectionSearchNeoflow::start_time() const {
	return _start_time;
}

inline TimeStamp ConnectionSearchNeoflow::end_time() const {
	return _end_time;
}

inline uint32_t ConnectionSearchNeoflow::byte_count() const {
	return _byte_count;
}

inline uint32_t ConnectionSearchNeoflow::packet_count() const {
	return _packet_count;
}

inline void ConnectionSearchNeoflow::neoflow_id(uint32_t neoflow_id) {
	_neoflow_id = neoflow_id;;
}

inline void ConnectionSearchNeoflow::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void ConnectionSearchNeoflow::source_ip(uint32_t source_ip) {
	_source_ip = ntohl(source_ip);
}

inline void ConnectionSearchNeoflow::destination_ip(uint32_t destination_ip) {
	_destination_ip = ntohl(destination_ip);
}

inline void ConnectionSearchNeoflow::source_port(uint16_t source_port) {
	_source_port = ntohs(source_port);
}

inline void ConnectionSearchNeoflow::destination_port(uint16_t destination_port) {
	_destination_port = ntohs(destination_port);
}

inline void ConnectionSearchNeoflow::raw_source_ip(uint32_t source_ip) {
	_source_ip = source_ip;
}

inline void ConnectionSearchNeoflow::raw_destination_ip(uint32_t destination_ip) {
	_destination_ip = destination_ip;
}

inline void ConnectionSearchNeoflow::raw_source_port(uint16_t source_port) {
	_source_port = source_port;
}

inline void ConnectionSearchNeoflow::raw_destination_port(uint16_t destination_port) {
	_destination_port = destination_port;
}

inline void ConnectionSearchNeoflow::start_time(const TimeStamp &start_time) {
	_start_time = start_time;
}

inline void ConnectionSearchNeoflow::end_time(const TimeStamp &end_time) {
	_end_time = end_time;
}

inline void ConnectionSearchNeoflow::byte_count(uint32_t byte_count) {
	_byte_count = byte_count;
}

inline void ConnectionSearchNeoflow::packet_count(uint32_t packet_count) {
	_packet_count = packet_count;
}

#endif
