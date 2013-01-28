#ifndef INFER_BIN_SEARCH_SEARCHNEOFLOWRESULT_HPP_
#define INFER_BIN_SEARCH_SEARCHNEOFLOWRESULT_HPP_

#include <sys/types.h>
#include <netinet/in.h>

#include "FlowStats.hpp"

class SearchNeoflowResult {
  public:
	SearchNeoflowResult();

	uint32_t index() const;
	uint8_t protocol() const;
	uint32_t sourceIP() const;
	uint32_t rawSourceIP() const;
	uint32_t destinationIP() const;
	uint32_t rawDestinationIP() const;
	uint16_t sourcePort() const;
	uint16_t rawSourcePort() const;
	uint16_t destinationPort() const;
	uint16_t rawDestinationPort() const;
	TimeStamp startTime() const;
	TimeStamp endTime() const;
	uint32_t numBytes() const;
	uint32_t numPackets() const;

	void index(uint32_t index);
	void protocol(uint8_t protocol);
	void sourceIP(uint32_t sourceIP);
	void rawSourceIP(uint32_t sourceIP);
	void destinationIP(uint32_t destinationIP);
	void rawDestinationIP(uint32_t destinationIP);
	void sourcePort(uint16_t sourcePort);
	void rawSourcePort(uint16_t sourcePort);
	void destinationPort(uint16_t destinationPort);
	void rawDestinationPort(uint16_t destinationPort);
	void startTime(const TimeStamp &startTime);
	void endTime(const TimeStamp &endTime);
	void numBytes(uint32_t numBytes);
	void numPackets(uint32_t numPackets);

	bool operator<(const SearchNeoflowResult &rhs) const;

  private:
	uint32_t _index;
	uint8_t _protocol;
	uint32_t _sourceIP;
	uint32_t _destinationIP;
	uint16_t _sourcePort;
	uint16_t _destinationPort;
	TimeStamp _startTime;
	TimeStamp _endTime;
	uint32_t _numBytes;
	uint32_t _numPackets;
};

inline SearchNeoflowResult::SearchNeoflowResult()
	:_index(0),
	 _protocol(0),
	 _sourceIP(0),
	 _destinationIP(0),
	 _sourcePort(0),
	 _destinationPort(0),
	 _startTime(),
	 _endTime(),
	 _numBytes(0),
	 _numPackets(0)
{
}

inline uint32_t SearchNeoflowResult::index() const {
	return _index;
}

inline uint8_t SearchNeoflowResult::protocol() const {
	return _protocol;
}

inline uint32_t SearchNeoflowResult::sourceIP() const {
	return ntohl(rawSourceIP());
}

inline uint32_t SearchNeoflowResult::destinationIP() const {
	return ntohl(rawDestinationIP());
}

inline uint16_t SearchNeoflowResult::sourcePort() const {
	return ntohs(rawSourcePort());
}

inline uint16_t SearchNeoflowResult::destinationPort() const {
	return ntohs(rawDestinationPort());
}

inline uint32_t SearchNeoflowResult::rawSourceIP() const {
	return _sourceIP;
}

inline uint32_t SearchNeoflowResult::rawDestinationIP() const {
	return _destinationIP;
}

inline uint16_t SearchNeoflowResult::rawSourcePort() const {
	return _sourcePort;
}

inline uint16_t SearchNeoflowResult::rawDestinationPort() const {
	return _destinationPort;
}

inline TimeStamp SearchNeoflowResult::startTime() const {
	return _startTime;
}

inline TimeStamp SearchNeoflowResult::endTime() const {
	return _endTime;
}

inline uint32_t SearchNeoflowResult::numBytes() const {
	return _numBytes;
}

inline uint32_t SearchNeoflowResult::numPackets() const {
	return _numPackets;
}

inline void SearchNeoflowResult::index(uint32_t index) {
	_index = index;;
}

inline void SearchNeoflowResult::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void SearchNeoflowResult::sourceIP(uint32_t sourceIP) {
	_sourceIP = ntohl(sourceIP);
}

inline void SearchNeoflowResult::destinationIP(uint32_t destinationIP) {
	_destinationIP = ntohl(destinationIP);
}

inline void SearchNeoflowResult::sourcePort(uint16_t sourcePort) {
	_sourcePort = ntohs(sourcePort);
}

inline void SearchNeoflowResult::destinationPort(uint16_t destinationPort) {
	_destinationPort = ntohs(destinationPort);
}

inline void SearchNeoflowResult::rawSourceIP(uint32_t sourceIP) {
	_sourceIP = sourceIP;
}

inline void SearchNeoflowResult::rawDestinationIP(uint32_t destinationIP) {
	_destinationIP = destinationIP;
}

inline void SearchNeoflowResult::rawSourcePort(uint16_t sourcePort) {
	_sourcePort = sourcePort;
}

inline void SearchNeoflowResult::rawDestinationPort(uint16_t destinationPort) {
	_destinationPort = destinationPort;
}

inline void SearchNeoflowResult::startTime(const TimeStamp &startTime) {
	_startTime = startTime;
}

inline void SearchNeoflowResult::endTime(const TimeStamp &endTime) {
	_endTime = endTime;
}

inline void SearchNeoflowResult::numBytes(uint32_t numBytes) {
	_numBytes = numBytes;
}

inline void SearchNeoflowResult::numPackets(uint32_t numPackets) {
	_numPackets = numPackets;
}

inline bool SearchNeoflowResult::operator<(const SearchNeoflowResult &rhs) const
{
	return _startTime < rhs._startTime;
}

#endif
