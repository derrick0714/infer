#ifndef INFER_INCLUDE_CONNECTION_HPP_
#define INFER_INCLUDE_CONNECTION_HPP_

#include <netinet/in.h>

#include "FlowStats.hpp"

class Connection {
  public:
	Connection()
		:_protocol(0),
		 _ip_a(0),
		 _ip_b(0),
		 _port_a(0),
		 _port_b(0),
		 _start_time(std::numeric_limits<uint32_t>::max(),
		 			 std::numeric_limits<uint32_t>::max()),
		 _end_time(0, 0),
		 _flows()
	{
	}

	Connection(uint8_t protocol,
		 uint32_t ip_a,
		 uint32_t ip_b,
		 uint16_t port_a,
		 uint16_t port_b)
		:_protocol(protocol),
		 _ip_a(ip_a),
		 _ip_b(ip_b),
		 _port_a(port_a),
		 _port_b(port_b),
		 _start_time(std::numeric_limits<uint32_t>::max(),
		 			 std::numeric_limits<uint32_t>::max()),
		 _end_time(0, 0),
		 _flows()
	{
	}

	uint8_t protocol() const {
		return _protocol;
	}

	uint32_t ip_a() const {
		return ntohl(_ip_a);
	}

	uint32_t ip_b() const {
		return ntohl(_ip_b);
	}

	uint16_t port_a() const {
		return ntohs(_port_a);
	}

	uint16_t port_b() const {
		return ntohs(_port_b);
	}

	uint32_t raw_ip_a() const {
		return _ip_a;
	}

	uint32_t raw_ip_b() const {
		return _ip_b;
	}

	uint16_t raw_port_a() const {
		return _port_a;
	}

	uint16_t raw_port_b() const {
		return _port_b;
	}

	TimeStamp start_time() const {
		return _start_time;
	}

	TimeStamp end_time() const {
		return _end_time;
	}

	std::vector<FlowStats> flows() const {
		return _flows;
	}

	void protocol(uint8_t protocol) {
		_protocol = protocol;
	}

	void ip_a(uint32_t ip_a) {
		_ip_a = ip_a;
	}

	void ip_b(uint32_t ip_b) {
		_ip_b = ip_b;
	}

	void port_a(uint16_t port_a) {
		_port_a = port_a;
	}

	void port_b(uint16_t port_b) {
		_port_b = port_b;
	}

	void raw_ip_a(uint32_t ip_a) {
		_ip_a = ip_a;
	}

	void raw_ip_b(uint32_t ip_b) {
		_ip_b = ip_b;
	}

	void raw_port_a(uint16_t port_a) {
		_port_a = port_a;
	}

	void raw_port_b(uint16_t port_b) {
		_port_b = port_b;
	}

	void start_time(const TimeStamp &start_time) {
		_start_time = start_time;
	}

	void end_time(const TimeStamp &end_time) {
		_end_time = end_time;
	}

	void flows(const std::vector<FlowStats> &flows) {
		_flows = flows;
	}

	void add_flow(const FlowStats &flow) {
		_flows.push_back(flow);
		if (flow.startTime() < _start_time) {
			_start_time = flow.startTime();
		}
		if (flow.endTime() > _end_time) {
			_end_time = flow.endTime();
		}
	}

  private:
	uint8_t _protocol;
	uint32_t _ip_a;
	uint32_t _ip_b;
	uint16_t _port_a;
	uint16_t _port_b;
	TimeStamp _start_time;
	TimeStamp _end_time;
	std::vector<FlowStats> _flows;
};

#endif
