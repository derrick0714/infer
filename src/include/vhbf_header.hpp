#ifndef INFER_INCLUDE_VHBF_HEADER_HPP_
#define INFER_INCLUDE_VHBF_HEADER_HPP_

#include <cstring>

#include <netinet/in.h>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"

class vhbf_header {
  public:
	vhbf_header()
		:_start_time(),
		 _end_time(),
		 _version(),
		 _max_payload(),
		 _num_insertions()
	{
		std::memset(_flow_id, 0, sizeof(_flow_id));
	}

	vhbf_header(const vhbf_header &h)
		:_start_time(h._start_time),
		 _end_time(h._end_time),
		 _version(h._version),
		 _max_payload(h._max_payload),
		 _num_insertions(h._num_insertions)
	{
		std::memcpy(_flow_id, h._flow_id, sizeof(_flow_id));
	}

	vhbf_header & operator= (const vhbf_header &h) {
		if (this == &h) {
			return *this;
		}

		_start_time = h._start_time;
		_end_time = h._end_time;
		std::memcpy(_flow_id, h._flow_id, sizeof(_flow_id));
		_version = h._version;
		_max_payload = h._max_payload;
		_num_insertions = h._num_insertions;

		return *this;
	}

	TimeStamp start_time() const {
		return _start_time;
	}

	void start_time(const TimeStamp &start_time) {
		_start_time = start_time;
	}

	TimeStamp end_time() const {
		return _end_time;
	}

	void end_time(const TimeStamp &end_time) {
		_end_time = end_time;
	}

	uint8_t protocol() const {
		return _protocol();
	}

	void protocol(uint8_t protocol) {
		_protocol() = protocol;
	}

	uint32_t source_ip() const {
		return ntohl(raw_source_ip());
	}

	uint32_t raw_source_ip() const {
		return _raw_source_ip();
	}

	void raw_source_ip(uint32_t raw_source_ip) {
		_raw_source_ip() = raw_source_ip;
	}

	
	uint32_t destination_ip() const {
		return ntohl(raw_destination_ip());
	}

	uint32_t raw_destination_ip() const {
		return _raw_destination_ip();
	}

	void raw_destination_ip(uint32_t raw_destination_ip) {
		_raw_destination_ip() = raw_destination_ip;
	}


	uint16_t source_port() const {
		return ntohs(raw_source_port());
	}

	uint16_t raw_source_port() const {
		return _raw_source_port();
	}

	void raw_source_port(uint16_t raw_source_port) {
		_raw_source_port() = raw_source_port;
	}


	uint16_t destination_port() const {
		return ntohs(raw_destination_port());
	}

	uint16_t raw_destination_port() const {
		return _raw_destination_port();
	}

	void raw_destination_port(uint16_t raw_destination_port) {
		_raw_destination_port() = raw_destination_port;
	}

	uint8_t version() const {
		return _version;
	}

	void version(uint8_t version) {
		_version = version;
	}

	uint16_t max_payload() const {
		return _max_payload;
	}

	void max_payload(uint16_t max_payload) {
		_max_payload = max_payload;
	}

	uint16_t num_insertions() const {
		return _num_insertions;
	}

	void num_insertions(uint16_t num_insertions) {
		_num_insertions = num_insertions;
	}

  private:
	uint8_t & _protocol() const {
		return *(reinterpret_cast<uint8_t *>(const_cast<char *>(_flow_id)));
	}

	uint32_t & _raw_source_ip() const {
		return *(reinterpret_cast<uint32_t *>(const_cast<char *>(_flow_id) + sizeof(_protocol())));
	}

	uint32_t & _raw_destination_ip() const {
		return *(reinterpret_cast<uint32_t *>(const_cast<char *>(_flow_id) +
					sizeof(_protocol()) +
					sizeof(_raw_source_ip())));
	}

	uint16_t & _raw_source_port() const {
		return *(reinterpret_cast<uint16_t *>(const_cast<char *>(_flow_id) +
					sizeof(_protocol()) +
					sizeof(_raw_source_ip()) +
					sizeof(_raw_destination_ip())));
	}

	uint16_t & _raw_destination_port() const {
		return *(reinterpret_cast<uint16_t *>(const_cast<char *>(_flow_id) +
					sizeof(_protocol()) +
					sizeof(_raw_source_ip()) +
					sizeof(_raw_destination_ip()) +
					sizeof(_raw_source_port())));
	}

	TimeStamp _start_time;
	TimeStamp _end_time;
	char _flow_id[13];
	uint8_t _version;
	uint16_t _max_payload;
	uint16_t _num_insertions;
};

#endif
