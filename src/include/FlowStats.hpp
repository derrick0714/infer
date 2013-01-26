#ifndef INFER_INCLUDE_FLOWSTATS_HPP_
#define INFER_INCLUDE_FLOWSTATS_HPP_

#include <sys/types.h>
#include <netinet/in.h>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"

class FlowStats {
  public:
	typedef plain_old_data_tag data_type;

	typedef enum {
		PLAINTEXT_TYPE,
		BMP_IMAGE_TYPE,
		WAV_AUDIO_TYPE,
		COMPRESSED_TYPE,
		JPEG_IMAGE_TYPE,
		MP3_AUDIO_TYPE,
		MPEG_VIDEO_TYPE,
		ENCRYPTED_TYPE,
		CONTENT_TYPES
	} ContentType;

	static const uint8_t TypeID = 0x01;
	static const size_t SizeDistRangeCount = 8;
	static const size_t SizeDistRangeWidth = 256;

	FlowStats();

	uint8_t version() const;
	uint8_t typeOfService() const;
	uint8_t protocol() const;
	uint32_t sourceIP() const;
	uint32_t rawSourceIP() const;
	uint32_t destinationIP() const;
	uint32_t rawDestinationIP() const;
	uint16_t sourcePort() const;
	uint16_t rawSourcePort() const;
	uint16_t destinationPort() const;
	uint16_t rawDestinationPort() const;
	TimeStamp time() const;
	TimeStamp startTime() const;
	TimeStamp firstSYNTime() const;
	TimeStamp firstSYNACKTime() const;
	TimeStamp firstACKTime() const;
	TimeStamp endTime() const;
	TimeStamp minInterArrivalTime() const;
	TimeStamp maxInterArrivalTime() const;
	uint8_t tcpFlags() const;
	uint8_t minTTL() const;
	uint8_t maxTTL() const;
	uint32_t numFrags() const;
	uint32_t numBytes() const;
	uint32_t numPackets() const;
	uint32_t minPacketSize() const;
	uint32_t maxPacketSize() const;
	uint32_t tcpURGs() const;
	uint32_t tcpACKs() const;
	uint32_t tcpPUSHs() const;
	uint32_t tcpRSTs() const;
	uint32_t tcpSYNs() const;
	uint32_t tcpFINs() const;
	uint32_t sizeDistribution(size_t index) const;
	uint32_t content(ContentType contentType) const;

	void version(uint8_t version);
	void typeOfService(uint8_t typeOfService);
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
	void firstSYNTime(const TimeStamp &firstSYNTime);
	void firstSYNACKTime(const TimeStamp &firstSYNACKTime);
	void firstACKTime(const TimeStamp &firstACKTime);
	void endTime(const TimeStamp &endTime);
	void minInterArrivalTime(const TimeStamp &minInterArrivalTime);
	void maxInterArrivalTime(const TimeStamp &maxInterArrivalTime);
	void tcpFlags(uint8_t tcpFlags);
	void minTTL(uint8_t minTTL);
	void maxTTL(uint8_t maxTTL);
	void numFrags(uint32_t numFrags);
	void numBytes(uint32_t numBytes);
	void numPackets(uint32_t numPackets);
	void minPacketSize(uint32_t minPacketSize);
	void maxPacketSize(uint32_t maxPacketSize);
	void tcpURGs(uint32_t tcpURGs);
	void tcpACKs(uint32_t tcpACKs);
	void tcpPUSHs(uint32_t tcpPUSHs);
	void tcpRSTs(uint32_t tcpRSTs);
	void tcpSYNs(uint32_t tcpSYNs);
	void tcpFINs(uint32_t tcpFINs);
	void sizeDistribution(size_t index, uint32_t value);
	void content(ContentType contentType, uint32_t amount);

	bool operator<(const FlowStats &rhs) const {
		return _startTime < rhs._startTime;
	}

  private:
	uint8_t _version;
	uint8_t _typeOfService;
	uint8_t _padding0;
	uint8_t _protocol;
	uint32_t _sourceIP;
	uint32_t _destinationIP;
	uint16_t _sourcePort;
	uint16_t _destinationPort;
	TimeStamp _startTime;
	TimeStamp _firstSYNTime;
	TimeStamp _firstSYNACKTime;
	TimeStamp _firstACKTime;
	TimeStamp _endTime;
	TimeStamp _minInterArrivalTime;
	TimeStamp _maxInterArrivalTime;
	uint8_t _tcpFlags;
	uint8_t _minTTL;
	uint8_t _maxTTL;
	uint8_t _padding1;
	uint32_t _numFrags;
	uint32_t _numBytes;
	uint32_t _numPackets;
	uint32_t _minPacketSize;
	uint32_t _maxPacketSize;
	uint32_t _tcpURGs;
	uint32_t _tcpACKs;
	uint32_t _tcpPUSHs;
	uint32_t _tcpRSTs;
	uint32_t _tcpSYNs;
	uint32_t _tcpFINs;
	uint32_t _sizeDistribution[SizeDistRangeCount];
	uint32_t _content[CONTENT_TYPES];
};

inline FlowStats::FlowStats()
	:_version(0),
	 _typeOfService(0),
	 _padding0(0),
	 _protocol(0),
	 _sourceIP(0),
	 _destinationIP(0),
	 _sourcePort(0),
	 _destinationPort(0),
	 _startTime(),
	 _firstSYNTime(),
	 _firstSYNACKTime(),
	 _firstACKTime(),
	 _endTime(),
	 _minInterArrivalTime(),
	 _maxInterArrivalTime(),
	 _tcpFlags(0),
	 _minTTL(0),
	 _maxTTL(0),
	 _padding1(0),
	 _numFrags(0),
	 _numBytes(0),
	 _numPackets(0),
	 _minPacketSize(0),
	 _maxPacketSize(0),
	 _tcpURGs(0),
	 _tcpACKs(0),
	 _tcpPUSHs(0),
	 _tcpRSTs(0),
	 _tcpSYNs(0),
	 _tcpFINs(0)
{
	for (size_t i(0); i < CONTENT_TYPES; ++i) {
		_content[i] = 0;
	}
	for (size_t i(0); i < SizeDistRangeCount; ++i) {
		_sizeDistribution[i] = 0;
	}
}

inline uint8_t FlowStats::version() const {
	return _version;
}

inline uint8_t FlowStats::typeOfService() const {
	return _typeOfService;
}

inline uint8_t FlowStats::protocol() const {
	return _protocol;
}

inline uint32_t FlowStats::sourceIP() const {
	return ntohl(rawSourceIP());
}

inline uint32_t FlowStats::destinationIP() const {
	return ntohl(rawDestinationIP());
}

inline uint16_t FlowStats::sourcePort() const {
	return ntohs(rawSourcePort());
}

inline uint16_t FlowStats::destinationPort() const {
	return ntohs(rawDestinationPort());
}

inline uint32_t FlowStats::rawSourceIP() const {
	return _sourceIP;
}

inline uint32_t FlowStats::rawDestinationIP() const {
	return _destinationIP;
}

inline uint16_t FlowStats::rawSourcePort() const {
	return _sourcePort;
}

inline uint16_t FlowStats::rawDestinationPort() const {
	return _destinationPort;
}

inline TimeStamp FlowStats::time() const {
	return _startTime;
}

inline TimeStamp FlowStats::startTime() const {
	return _startTime;
}

inline TimeStamp FlowStats::firstSYNTime() const {
	return _firstSYNTime;
}

inline TimeStamp FlowStats::firstSYNACKTime() const {
	return _firstSYNACKTime;
}

inline TimeStamp FlowStats::firstACKTime() const {
	return _firstACKTime;
}

inline TimeStamp FlowStats::endTime() const {
	return _endTime;
}

inline TimeStamp FlowStats::minInterArrivalTime() const {
	return _minInterArrivalTime;
}

inline TimeStamp FlowStats::maxInterArrivalTime() const {
	return _maxInterArrivalTime;
}

inline uint8_t FlowStats::tcpFlags() const {
	return _tcpFlags;
}

inline uint8_t FlowStats::minTTL() const {
	return _minTTL;
}

inline uint8_t FlowStats::maxTTL() const {
	return _maxTTL;
}

inline uint32_t FlowStats::numFrags() const {
	return _numFrags;
}

inline uint32_t FlowStats::numBytes() const {
	return _numBytes;
}

inline uint32_t FlowStats::numPackets() const {
	return _numPackets;
}

inline uint32_t FlowStats::minPacketSize() const {
	return _minPacketSize;
}

inline uint32_t FlowStats::maxPacketSize() const {
	return _maxPacketSize;
}

inline uint32_t FlowStats::tcpURGs() const {
	return _tcpURGs;
}

inline uint32_t FlowStats::tcpACKs() const {
	return _tcpACKs;
}

inline uint32_t FlowStats::tcpPUSHs() const {
	return _tcpPUSHs;
}

inline uint32_t FlowStats::tcpRSTs() const {
	return _tcpRSTs;
}

inline uint32_t FlowStats::tcpSYNs() const {
	return _tcpSYNs;
}

inline uint32_t FlowStats::tcpFINs() const {
	return _tcpFINs;
}

inline uint32_t FlowStats::sizeDistribution(size_t index) const {
	return _sizeDistribution[index];
}

inline uint32_t FlowStats::content(FlowStats::ContentType contentType) const {
	return _content[contentType];
}

inline void FlowStats::version(uint8_t version) {
	_version = version;
}

inline void FlowStats::typeOfService(uint8_t typeOfService) {
	_typeOfService = typeOfService;
}

inline void FlowStats::protocol(uint8_t protocol) {
	_protocol = protocol;
}

inline void FlowStats::sourceIP(uint32_t sourceIP) {
	_sourceIP = ntohl(sourceIP);
}

inline void FlowStats::destinationIP(uint32_t destinationIP) {
	_destinationIP = ntohl(destinationIP);
}

inline void FlowStats::sourcePort(uint16_t sourcePort) {
	_sourcePort = ntohs(sourcePort);
}

inline void FlowStats::destinationPort(uint16_t destinationPort) {
	_destinationPort = ntohs(destinationPort);
}

inline void FlowStats::rawSourceIP(uint32_t sourceIP) {
	_sourceIP = sourceIP;
}

inline void FlowStats::rawDestinationIP(uint32_t destinationIP) {
	_destinationIP = destinationIP;
}

inline void FlowStats::rawSourcePort(uint16_t sourcePort) {
	_sourcePort = sourcePort;
}

inline void FlowStats::rawDestinationPort(uint16_t destinationPort) {
	_destinationPort = destinationPort;
}

inline void FlowStats::startTime(const TimeStamp &startTime) {
	_startTime = startTime;
}

inline void FlowStats::firstSYNTime(const TimeStamp &firstSYNTime) {
	_firstSYNTime = firstSYNTime;
}

inline void FlowStats::firstSYNACKTime(const TimeStamp &firstSYNACKTime) {
	_firstSYNACKTime = firstSYNACKTime;
}

inline void FlowStats::firstACKTime(const TimeStamp &firstACKTime) {
	_firstACKTime = firstACKTime;
}

inline void FlowStats::endTime(const TimeStamp &endTime) {
	_endTime = endTime;
}

inline void FlowStats::minInterArrivalTime(const TimeStamp &minInterArrivalTime) {
	_minInterArrivalTime = minInterArrivalTime;
}

inline void FlowStats::maxInterArrivalTime(const TimeStamp &maxInterArrivalTime) {
	_maxInterArrivalTime = maxInterArrivalTime;
}

inline void FlowStats::tcpFlags(uint8_t tcpFlags) {
	_tcpFlags = tcpFlags;
}

inline void FlowStats::minTTL(uint8_t minTTL) {
	_minTTL = minTTL;
}

inline void FlowStats::maxTTL(uint8_t maxTTL) {
	_maxTTL = maxTTL;
}

inline void FlowStats::numFrags(uint32_t numFrags) {
	_numFrags = numFrags;
}

inline void FlowStats::numBytes(uint32_t numBytes) {
	_numBytes = numBytes;
}

inline void FlowStats::numPackets(uint32_t numPackets) {
	_numPackets = numPackets;
}

inline void FlowStats::minPacketSize(uint32_t minPacketSize) {
	_minPacketSize = minPacketSize;
}

inline void FlowStats::maxPacketSize(uint32_t maxPacketSize) {
	_maxPacketSize = maxPacketSize;
}

inline void FlowStats::tcpURGs(uint32_t tcpURGs) {
	_tcpURGs = tcpURGs;
}

inline void FlowStats::tcpACKs(uint32_t tcpACKs) {
	_tcpACKs = tcpACKs;
}

inline void FlowStats::tcpPUSHs(uint32_t tcpPUSHs) {
	_tcpPUSHs = tcpPUSHs;
}

inline void FlowStats::tcpRSTs(uint32_t tcpRSTs) {
	_tcpRSTs = tcpRSTs;
}

inline void FlowStats::tcpSYNs(uint32_t tcpSYNs) {
	_tcpSYNs = tcpSYNs;
}

inline void FlowStats::tcpFINs(uint32_t tcpFINs) {
	_tcpFINs = tcpFINs;
}

inline void FlowStats::sizeDistribution(size_t index, uint32_t value) {
	_sizeDistribution[index] = value;
}

inline void FlowStats::content(FlowStats::ContentType contentType,
							   uint32_t amount)
{
	_content[contentType] = amount;
}

#endif
