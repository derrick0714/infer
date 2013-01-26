#ifndef HBFRESULT_HPP
#define HBFRESULT_HPP

#include <string>
#include <netinet/in.h>

#include "TimeStamp.h"

namespace vn {
namespace arl {
namespace shared {

class HBFResult {
  public:
	HBFResult()
		:_startTime(),
		 _endTime(),
		 _match()
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
	
	/// \brief Get the start time
	/// \returns the start time
	TimeStamp startTime() const;

	/// \brief Set the start time
	/// \param startTime the start time to set
	void startTime(const TimeStamp &startTime);

	/// \brief Get the end time
	/// \returns the end time
	TimeStamp endTime() const;

	/// \brief Set the end time
	/// \param endTime the end time to set
	void endTime(const TimeStamp &endTime);

	/// \brief Get the matching data
	/// \returns the matching data
	std::string match() const;

	/// \brief Set the matching data
	/// \param match the matching data to set
	void match(const std::string &match);

	bool operator<(const HBFResult &rhs) const;

  private:
	char _flowID[13];
	TimeStamp _startTime;
	TimeStamp _endTime;
	std::string _match;
};

inline uint8_t HBFResult::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void HBFResult::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t HBFResult::sourceIP() const {
	return ntohl(rawSourceIP());
}

inline void HBFResult::sourceIP(uint32_t sourceIP) {
	rawSourceIP(htonl(sourceIP));
}

inline uint32_t HBFResult::rawSourceIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void HBFResult::rawSourceIP(uint32_t sourceIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= sourceIP;
}

inline uint32_t HBFResult::destinationIP() const {
	return ntohl(rawDestinationIP());
}

inline void HBFResult::destinationIP(uint32_t destinationIP) {
	rawDestinationIP(htonl(destinationIP));
}

inline uint32_t HBFResult::rawDestinationIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())));
}

inline void HBFResult::rawDestinationIP(uint32_t destinationIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())))
		= destinationIP;
}

inline uint16_t HBFResult::sourcePort() const {
	return ntohs(rawSourcePort());
}

inline void HBFResult::sourcePort(uint16_t sourcePort) {
	rawSourcePort(htons(sourcePort));
}

inline uint16_t HBFResult::rawSourcePort() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())));
}

inline void HBFResult::rawSourcePort(uint16_t sourcePort) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())))
		= sourcePort;
}

inline uint16_t HBFResult::destinationPort() const {
	return ntohs(rawDestinationPort());
}

inline void HBFResult::destinationPort(uint16_t destinationPort) {
	rawDestinationPort(htons(destinationPort));
}

inline uint16_t HBFResult::rawDestinationPort() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())));
}

inline void HBFResult::rawDestinationPort(uint16_t destinationPort) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())))
		= destinationPort;
}

inline TimeStamp HBFResult::startTime() const {
	return _startTime;
}

inline void HBFResult::startTime(const TimeStamp &startTime) {
	_startTime = startTime;
}

inline TimeStamp HBFResult::endTime() const {
	return _endTime;
}

inline void HBFResult::endTime(const TimeStamp &endTime) {
	_endTime = endTime;
}

inline std::string HBFResult::match() const {
	return _match;
}

inline void HBFResult::match(const std::string &match) {
	_match = match;
}

inline bool HBFResult::operator<(const HBFResult &rhs) const {
	return _startTime < rhs._startTime;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
