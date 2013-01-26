#ifndef INFER_INCLUDE_HTTPRESPONSE_H
#define INFER_INCLUDE_HTTPRESPONSE_H

#include <string>
#include <netinet/in.h>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

class HTTPResponse {
  public:
	typedef serializable_data_tag data_type;

	static const uint8_t TypeID = 0x94;

	HTTPResponse()
		:_time(),
		 _version(),
		 _status(),
		 _reason(),
		 _response(),
		 _content_type()
	{
	}

	/// \brief Get the protocol number
	/// \returns the protocol number
	uint8_t protocol() const;

	/// \brief Set the protocol number
	/// \param protocol the protocol number to set
	void protocol(uint8_t protocol);

	/// \brief Get the source IPv4 address in host byte order
	/// \returns the source IPv4 address in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint32_t source_ip() const;

	/// \brief Set the source IPv4 address
	/// \param source_ip the IPv4 address to set, in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void source_ip(uint32_t source_ip);

	/// \brief Get the source IPv4 address in network byte order
	/// \returns the source IPv4 address in network byte order
	uint32_t raw_source_ip() const;

	/// \brief Set the source IPv4 address
	/// \param the source IPv4 address to set, in network byte order
	void raw_source_ip(uint32_t source_ip);

	/// \brief Get the destination IPv4 address in host byte order
	/// \returns the destination IPv4 address in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint32_t destination_ip() const;

	/// \brief Set the destination IPv4 address
	/// \param destination_ip the IPv4 address to set, in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void destination_ip(uint32_t destination_ip);

	/// \brief Get the destination IPv4 address in network byte order
	/// \returns the destination IPv4 address in network byte order
	uint32_t raw_destination_ip() const;

	/// \brief Set the destination IPv4 address
	/// \param the destination IPv4 address to set, in network byte order
	void raw_destination_ip(uint32_t destination_ip);

	/// \brief Get the source port number in host byte order
	/// \returns the source port number in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint16_t source_port() const;

	/// \brief Set the source port number
	/// \param source_ip the port number to set, in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void source_port(uint16_t source_port);

	/// \brief Get the source port number in network byte order
	/// \returns the source port number in network byte order
	uint16_t raw_source_port() const;

	/// \brief Set the source port number
	/// \param the source port number to set, in network byte order
	void raw_source_port(uint16_t source_port);

	/// \brief Get the destination port number in host byte order
	/// \returns the destination port number in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint16_t destination_port() const;

	/// \brief Set the destination port number
	/// \param destination_ip the port number to set, in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void destination_port(uint16_t destination_port);

	/// \brief Get the destination port number in network byte order
	/// \returns the destination port number in network byte order
	uint16_t raw_destination_port() const;

	/// \brief Set the destination port number
	/// \param the destination port number to set, in network byte order
	void raw_destination_port(uint16_t destination_port);
	
	/// \brief Get the time
	/// \returns the time
	TimeStamp time() const;

	/// \brief Set the time
	/// \param time the time to set
	void time(const TimeStamp &time);

	std::string version() const;
	std::string status() const;
	std::string reason() const;
	std::string response() const;
	std::string content_type() const;

	void version(const std::string &version);
	void status(const std::string &status);
	void reason(const std::string &reason);
	void response(const std::string &response);
	void content_type(const std::string &content_type);

	ErrorStatus serialize(std::ostream &dest) const;
	ErrorStatus unserialize(std::istream &src);

  private:
	char _flowID[13];
	TimeStamp _time;
	std::string _version;
	std::string _status;
	std::string _reason;
	std::string _response;
	std::string _content_type;
};

inline uint8_t HTTPResponse::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void HTTPResponse::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t HTTPResponse::source_ip() const {
	return ntohl(raw_source_ip());
}

inline void HTTPResponse::source_ip(uint32_t source_ip) {
	raw_source_ip(htonl(source_ip));
}

inline uint32_t HTTPResponse::raw_source_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void HTTPResponse::raw_source_ip(uint32_t source_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= source_ip;
}

inline uint32_t HTTPResponse::destination_ip() const {
	return ntohl(raw_destination_ip());
}

inline void HTTPResponse::destination_ip(uint32_t destination_ip) {
	raw_destination_ip(htonl(destination_ip));
}

inline uint32_t HTTPResponse::raw_destination_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())));
}

inline void HTTPResponse::raw_destination_ip(uint32_t destination_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())))
		= destination_ip;
}

inline uint16_t HTTPResponse::source_port() const {
	return ntohs(raw_source_port());
}

inline void HTTPResponse::source_port(uint16_t source_port) {
	raw_source_port(htons(source_port));
}

inline uint16_t HTTPResponse::raw_source_port() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())));
}

inline void HTTPResponse::raw_source_port(uint16_t source_port) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())))
		= source_port;
}

inline uint16_t HTTPResponse::destination_port() const {
	return ntohs(raw_destination_port());
}

inline void HTTPResponse::destination_port(uint16_t destination_port) {
	raw_destination_port(htons(destination_port));
}

inline uint16_t HTTPResponse::raw_destination_port() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())));
}

inline void HTTPResponse::raw_destination_port(uint16_t destination_port) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())))
		= destination_port;
}

inline TimeStamp HTTPResponse::time() const {
	return _time;
}

inline void HTTPResponse::time(const TimeStamp &time) {
	_time = time;
}

inline std::string HTTPResponse::version() const {
	return _version;
}

inline std::string HTTPResponse::status() const {
	return _status;
}

inline std::string HTTPResponse::reason() const {
	return _reason;
}

inline std::string HTTPResponse::response() const {
	return _response;
}

inline std::string HTTPResponse::content_type() const {
	return _content_type;
}

inline void HTTPResponse::version(const std::string &version) {
	_version = version;
}

inline void HTTPResponse::status(const std::string &status) {
	_status = status;
}

inline void HTTPResponse::reason(const std::string &reason) {
	_reason = reason;
}

inline void HTTPResponse::response(const std::string &response) {
	_response = response;
}

inline void HTTPResponse::content_type(const std::string &content_type) {
	_content_type = content_type;
}

#endif
