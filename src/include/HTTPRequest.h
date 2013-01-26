#ifndef INFER_INCLUDE_HTTPREQUSET_H_
#define INFER_INCLUDE_HTTPREQUSET_H_

#include <string>
#include <netinet/in.h>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

class HTTPRequest {
  public:
	typedef serializable_data_tag data_type;

	static const uint8_t TypeID = 0x93;

	HTTPRequest()
		:_time(),
		 _type(),
		 _uri(),
		 _version(),
		 _host(),
		 _user_agent(),
		 _referer()
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

	std::string type() const;
	std::string uri() const;
	std::string version() const;
	std::string host() const;
	std::string user_agent() const;
	std::string referer() const;

	void type(const std::string &type);
	void uri(const std::string &uri);
	void version(const std::string &version);
	void host(const std::string &host);
	void user_agent(const std::string &user_agent);
	void referer(const std::string &referer);

	ErrorStatus serialize(std::ostream &dest) const;
	ErrorStatus unserialize(std::istream &src);

  private:
	char _flowID[13];
	TimeStamp _time;
	std::string _type;
	std::string _uri;
	std::string _version;
	std::string _host;
	std::string _user_agent;
	std::string _referer;
};

inline uint8_t HTTPRequest::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void HTTPRequest::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t HTTPRequest::source_ip() const {
	return ntohl(raw_source_ip());
}

inline void HTTPRequest::source_ip(uint32_t source_ip) {
	raw_source_ip(htonl(source_ip));
}

inline uint32_t HTTPRequest::raw_source_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void HTTPRequest::raw_source_ip(uint32_t source_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= source_ip;
}

inline uint32_t HTTPRequest::destination_ip() const {
	return ntohl(raw_destination_ip());
}

inline void HTTPRequest::destination_ip(uint32_t destination_ip) {
	raw_destination_ip(htonl(destination_ip));
}

inline uint32_t HTTPRequest::raw_destination_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())));
}

inline void HTTPRequest::raw_destination_ip(uint32_t destination_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())))
		= destination_ip;
}

inline uint16_t HTTPRequest::source_port() const {
	return ntohs(raw_source_port());
}

inline void HTTPRequest::source_port(uint16_t source_port) {
	raw_source_port(htons(source_port));
}

inline uint16_t HTTPRequest::raw_source_port() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())));
}

inline void HTTPRequest::raw_source_port(uint16_t source_port) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())))
		= source_port;
}

inline uint16_t HTTPRequest::destination_port() const {
	return ntohs(raw_destination_port());
}

inline void HTTPRequest::destination_port(uint16_t destination_port) {
	raw_destination_port(htons(destination_port));
}

inline uint16_t HTTPRequest::raw_destination_port() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())));
}

inline void HTTPRequest::raw_destination_port(uint16_t destination_port) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())))
		= destination_port;
}

inline TimeStamp HTTPRequest::time() const {
	return _time;
}

inline void HTTPRequest::time(const TimeStamp &time) {
	_time = time;
}

inline std::string HTTPRequest::type() const {
	return _type;
}

inline std::string HTTPRequest::uri() const {
	return _uri;
}

inline std::string HTTPRequest::version() const {
	return _version;
}

inline std::string HTTPRequest::host() const {
	return _host;
}

inline std::string HTTPRequest::user_agent() const {
	return _user_agent;
}

inline std::string HTTPRequest::referer() const {
	return _referer;
}

inline void HTTPRequest::type(const std::string &type) {
	_type = type;
}

inline void HTTPRequest::uri(const std::string &uri) {
	_uri = uri;
}

inline void HTTPRequest::version(const std::string &version) {
	_version = version;
}

inline void HTTPRequest::host(const std::string &host) {
	_host = host;
}

inline void HTTPRequest::user_agent(const std::string &user_agent) {
	_user_agent = user_agent;
}

inline void HTTPRequest::referer(const std::string &referer) {
	_referer = referer;
}

#endif
