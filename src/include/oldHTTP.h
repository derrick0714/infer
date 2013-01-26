#ifndef INFER_INCLUDE_HTTP_H
#define INFER_INCLUDE_HTTP_H

#include <string>
#include <netinet/in.h>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

class OldHTTP {
  public:
	typedef serializable_data_tag data_type;

	static const uint8_t TypeID = 0x91;

	OldHTTP()
		:_time(),
		 _version(),
		 _type(),
		 _requestType(),
		 _uri(),
		 _host(),
		 _userAgent(),
		 _referer(),
		 _status(),
		 _response(),
		 _reason(),
		 _contentType()
	{
	}

	/// \brief Clear this OldHTTP
	void clear();

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
	uint32_t sourceIP() const;

	/// \brief Set the source IPv4 address
	/// \param sourceIP the IPv4 address to set, in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void sourceIP(uint32_t sourceIP);

	/// \brief Get the source IPv4 address in network byte order
	/// \returns the source IPv4 address in network byte order
	uint32_t rawSourceIP() const;

	/// \brief Set the source IPv4 address
	/// \param the source IPv4 address to set, in network byte order
	void rawSourceIP(uint32_t sourceIP);

	/// \brief Get the destination IPv4 address in host byte order
	/// \returns the destination IPv4 address in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint32_t destinationIP() const;

	/// \brief Set the destination IPv4 address
	/// \param destinationIP the IPv4 address to set, in host byte order
	/// \note this class stores all IP addresses in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void destinationIP(uint32_t destinationIP);

	/// \brief Get the destination IPv4 address in network byte order
	/// \returns the destination IPv4 address in network byte order
	uint32_t rawDestinationIP() const;

	/// \brief Set the destination IPv4 address
	/// \param the destination IPv4 address to set, in network byte order
	void rawDestinationIP(uint32_t destinationIP);

	/// \brief Get the source port number in host byte order
	/// \returns the source port number in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint16_t sourcePort() const;

	/// \brief Set the source port number
	/// \param sourceIP the port number to set, in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void sourcePort(uint16_t sourcePort);

	/// \brief Get the source port number in network byte order
	/// \returns the source port number in network byte order
	uint16_t rawSourcePort() const;

	/// \brief Set the source port number
	/// \param the source port number to set, in network byte order
	void rawSourcePort(uint16_t sourcePort);

	/// \brief Get the destination port number in host byte order
	/// \returns the destination port number in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	uint16_t destinationPort() const;

	/// \brief Set the destination port number
	/// \param destinationIP the port number to set, in host byte order
	/// \note this class stores all port numbers in network byte order, so
	/// calling this function results in a byte-swap on litte-endian systems
	void destinationPort(uint16_t destinationPort);

	/// \brief Get the destination port number in network byte order
	/// \returns the destination port number in network byte order
	uint16_t rawDestinationPort() const;

	/// \brief Set the destination port number
	/// \param the destination port number to set, in network byte order
	void rawDestinationPort(uint16_t destinationPort);
	
	/// \brief Get the time
	/// \returns the time
	TimeStamp time() const;

	/// \brief Set the time
	/// \param time the time to set
	void time(const TimeStamp &time);

	std::string version() const;
	char type() const;
	std::string requestType() const;
	std::string uri() const;
	std::string host() const;
	std::string userAgent() const;
	std::string referer() const;
	std::string status() const;
	std::string response() const;
	std::string reason() const;
	std::string contentType() const;

	void version(const std::string &version);
	void type(char type);
	void requestType(const std::string &requestType);
	void uri(const std::string &uri);
	void host(const std::string &host);
	void userAgent(const std::string &userAgent);
	void referer(const std::string &referer);
	void status(const std::string &status);
	void response(const std::string &response);
	void reason(const std::string &reason);
	void contentType(const std::string &contentType);

	ErrorStatus serialize(std::ostream &dest) const;
	ErrorStatus unserialize(std::istream &src);

  private:
	char _flowID[13];
	TimeStamp _time;
	std::string _version;
	char _type;
	std::string _requestType;
	std::string _uri;
	std::string _host;
	std::string _userAgent;
	std::string _referer;
	std::string _status;
	std::string _response;
	std::string _reason;
	std::string _contentType;
};

inline void OldHTTP::clear() {
	memset(_flowID, 0, sizeof(_flowID));
	_time.set(0, 0);
	_version.clear();
	_type = 0;
	_requestType.clear();
	_uri.clear();
	_host.clear();
	_userAgent.clear();
	_referer.clear();
	_status.clear();
	_response.clear();
	_reason.clear();
	_contentType.clear();
}

inline uint8_t OldHTTP::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void OldHTTP::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t OldHTTP::sourceIP() const {
	return ntohl(rawSourceIP());
}

inline void OldHTTP::sourceIP(uint32_t sourceIP) {
	rawSourceIP(htonl(sourceIP));
}

inline uint32_t OldHTTP::rawSourceIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void OldHTTP::rawSourceIP(uint32_t sourceIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= sourceIP;
}

inline uint32_t OldHTTP::destinationIP() const {
	return ntohl(rawDestinationIP());
}

inline void OldHTTP::destinationIP(uint32_t destinationIP) {
	rawDestinationIP(htonl(destinationIP));
}

inline uint32_t OldHTTP::rawDestinationIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())));
}

inline void OldHTTP::rawDestinationIP(uint32_t destinationIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())))
		= destinationIP;
}

inline uint16_t OldHTTP::sourcePort() const {
	return ntohs(rawSourcePort());
}

inline void OldHTTP::sourcePort(uint16_t sourcePort) {
	rawSourcePort(htons(sourcePort));
}

inline uint16_t OldHTTP::rawSourcePort() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())));
}

inline void OldHTTP::rawSourcePort(uint16_t sourcePort) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())))
		= sourcePort;
}

inline uint16_t OldHTTP::destinationPort() const {
	return ntohs(rawDestinationPort());
}

inline void OldHTTP::destinationPort(uint16_t destinationPort) {
	rawDestinationPort(htons(destinationPort));
}

inline uint16_t OldHTTP::rawDestinationPort() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())));
}

inline void OldHTTP::rawDestinationPort(uint16_t destinationPort) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())))
		= destinationPort;
}

inline TimeStamp OldHTTP::time() const {
	return _time;
}

inline void OldHTTP::time(const TimeStamp &time) {
	_time = time;
}

inline std::string OldHTTP::version() const {
	return _version;
}

inline char OldHTTP::type() const {
	return _type;
}

inline std::string OldHTTP::requestType() const {
	return _requestType;
}

inline std::string OldHTTP::uri() const {
	return _uri;
}

inline std::string OldHTTP::host() const {
	return _host;
}

inline std::string OldHTTP::userAgent() const {
	return _userAgent;
}

inline std::string OldHTTP::referer() const {
	return _referer;
}

inline std::string OldHTTP::status() const {
	return _status;
}

inline std::string OldHTTP::response() const {
	return _response;
}

inline std::string OldHTTP::reason() const {
	return _reason;
}

inline std::string OldHTTP::contentType() const {
	return _contentType;
}

inline void OldHTTP::version(const std::string &version) {
	_version = version;
}

inline void OldHTTP::type(char type) {
	_type = type;
}

inline void OldHTTP::requestType(const std::string &requestType) {
	_requestType = requestType;
}

inline void OldHTTP::uri(const std::string &uri) {
	_uri = uri;
}

inline void OldHTTP::host(const std::string &host) {
	_host = host;
}

inline void OldHTTP::userAgent(const std::string &userAgent) {
	_userAgent = userAgent;
}

inline void OldHTTP::referer(const std::string &referer) {
	_referer = referer;
}

inline void OldHTTP::status(const std::string &status) {
	_status = status;
}

inline void OldHTTP::response(const std::string &response) {
	_response = response;
}

inline void OldHTTP::reason(const std::string &reason) {
	_reason = reason;
}

inline void OldHTTP::contentType(const std::string &contentType) {
	_contentType = contentType;
}

#endif
