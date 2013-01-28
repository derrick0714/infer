#ifndef INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHCONNECTION_HPP_
#define INFER_BIN_CONNECTIONSEARCH_CONNECTIONSEARCHCONNECTION_HPP_

#include <sys/types.h>
#include <netinet/in.h>

#include "timeStamp.h"

class ConnectionSearchConnection {
  public:
	ConnectionSearchConnection();
	ConnectionSearchConnection(uint32_t connection_id,
							   uint8_t protocol,
							   uint32_t ip_a,
							   uint32_t ip_b,
							   uint16_t port_a,
							   uint16_t port_b);

	uint32_t connection_id() const;
	TimeStamp start_time() const;
	TimeStamp end_time() const;
	uint8_t protocol() const;
	uint32_t ip_a() const;
	uint32_t raw_ip_a() const;
	uint32_t ip_b() const;
	uint32_t raw_ip_b() const;
	uint16_t port_a() const;
	uint16_t raw_port_a() const;
	uint16_t port_b() const;
	uint16_t raw_port_b() const;

	void connection_id(uint32_t connection_id);
	void protocol(uint8_t protocol);
	void start_time(const TimeStamp &start_time);
	void end_time(const TimeStamp &end_time);
	void ip_a(uint32_t ip_a);
	void raw_ip_a(uint32_t ip_a);
	void ip_b(uint32_t ip_b);
	void raw_ip_b(uint32_t ip_b);
	void port_a(uint16_t port_a);
	void raw_port_a(uint16_t port_a);
	void port_b(uint16_t port_b);
	void raw_port_b(uint16_t port_b);

	bool operator<(const ConnectionSearchConnection &rhs) const {
		return _start_time < rhs._start_time;
	}

  private:
	uint32_t _connection_id;
	TimeStamp _start_time;
	TimeStamp _end_time;
	uint8_t _protocol;
	uint32_t _ip_a;
	uint32_t _ip_b;
	uint16_t _port_a;
	uint16_t _port_b;
};

inline ConnectionSearchConnection::ConnectionSearchConnection()
	:_connection_id(0),
	 _start_time(std::numeric_limits<uint32_t>::max(),
	 			 std::numeric_limits<uint32_t>::max()),
	 _end_time(0, 0),
	 _protocol(0),
	 _ip_a(0),
	 _ip_b(0),
	 _port_a(0),
	 _port_b(0)
{
}

inline ConnectionSearchConnection::ConnectionSearchConnection(
											uint32_t connection_id,
											uint8_t protocol,
											uint32_t ip_a,
											uint32_t ip_b,
											uint16_t port_a,
											uint16_t port_b)
	:_connection_id(connection_id),
	 _start_time(std::numeric_limits<uint32_t>::max(),
	 			 std::numeric_limits<uint32_t>::max()),
	 _end_time(0, 0),
	 _protocol(protocol),
	 _ip_a(ip_a),
	 _ip_b(ip_b),
	 _port_a(port_a),
	 _port_b(port_b)
{
}

inline uint32_t ConnectionSearchConnection::connection_id() const {
	return _connection_id;
}

inline TimeStamp ConnectionSearchConnection::start_time() const {
	return _start_time;
}

inline TimeStamp ConnectionSearchConnection::end_time() const {
	return _end_time;
}

inline uint8_t ConnectionSearchConnection::protocol() const {
	return _protocol;
}

inline uint32_t ConnectionSearchConnection::ip_a() const {
	return ntohl(raw_ip_a());
}

inline uint32_t ConnectionSearchConnection::ip_b() const {
	return ntohl(raw_ip_b());
}

inline uint16_t ConnectionSearchConnection::port_a() const {
	return ntohs(raw_port_a());
}

inline uint16_t ConnectionSearchConnection::port_b() const {
	return ntohs(raw_port_b());
}

inline uint32_t ConnectionSearchConnection::raw_ip_a() const {
	return _ip_a;
}

inline uint32_t ConnectionSearchConnection::raw_ip_b() const {
	return _ip_b;
}

inline uint16_t ConnectionSearchConnection::raw_port_a() const {
	return _port_a;
}

inline uint16_t ConnectionSearchConnection::raw_port_b() const {
	return _port_b;
}

inline void ConnectionSearchConnection::connection_id(uint32_t connection_id) {
	_connection_id = connection_id;;
}

inline void ConnectionSearchConnection::start_time(const TimeStamp &start_time) {
	_start_time = start_time;
}

inline void ConnectionSearchConnection::end_time(const TimeStamp &end_time) {
	_end_time = end_time;
}

inline void ConnectionSearchConnection::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void ConnectionSearchConnection::ip_a(uint32_t ip_a) {
	_ip_a = ntohl(ip_a);
}

inline void ConnectionSearchConnection::ip_b(uint32_t ip_b) {
	_ip_b = ntohl(ip_b);
}

inline void ConnectionSearchConnection::port_a(uint16_t port_a) {
	_port_a = ntohs(port_a);
}

inline void ConnectionSearchConnection::port_b(uint16_t port_b) {
	_port_b = ntohs(port_b);
}

inline void ConnectionSearchConnection::raw_ip_a(uint32_t ip_a) {
	_ip_a = ip_a;
}

inline void ConnectionSearchConnection::raw_ip_b(uint32_t ip_b) {
	_ip_b = ip_b;
}

inline void ConnectionSearchConnection::raw_port_a(uint16_t port_a) {
	_port_a = port_a;
}

inline void ConnectionSearchConnection::raw_port_b(uint16_t port_b) {
	_port_b = port_b;
}

#endif
