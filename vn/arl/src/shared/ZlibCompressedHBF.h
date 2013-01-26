#ifndef ZLIBCOMPRESSEDHBF_H
#define ZLIBCOMPRESSEDHBF_H

#include <bitset>
#include <netinet/in.h>

#include "HBF.h"
#include "DataTypeTraits.hpp"
#include "TimeStamp.h"
#include "ZlibCompressedData.hpp"

namespace vn {
namespace arl {
namespace shared {

class HBF;

class ZlibCompressedHBF {
  public:
	typedef blob_data_tag data_type;

	/// \brief the maximum size of a ZlibCompressedHBF
	static const size_t DataSize;

	enum {
		Version = 2,     /// The HBF Version Number
		HBFSize = 32768, /// The size of the HBF bitset
		BlockSize = 64,  /// The block size
		NumHashes = 2    /// The number of hashes
	};

	/// \brief Default constructor
	ZlibCompressedHBF();

	/// \brief Construct with time and payload size
	/// \param flowID a pointer to the 13 byte flow ID
	/// \param payloadSize the first payloadSize
	/// \param t the first time
	///
	/// Constructs a new ZlibCompressedHBF, setting the flow ID to flowID,
	/// start and end times to t, and the size of the largest payload to 
	/// payloadSize
	ZlibCompressedHBF(const char *flowID, uint16_t payloadSize, const TimeStamp &t);

	/// \brief Construct from HBF
	/// \param hbf the HBF to construct from
	///
	/// Constructs a new ZlibCompressedHBF from an HBF
	ZlibCompressedHBF(const HBF &hbf);

	/// \brief Get the ZlibCompressedHBF version number
	/// \returns the ZlibCompressedHBF version number
	uint8_t version() const;

	/// \brief Set the ZlibCompressedHBF version number
	/// \param version the version number to set
	void version(uint8_t version);

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

	/// \brief Get the maximum payload size
	/// \returns the maximum payload size
	uint16_t maxPayload() const;

	/// \brief Set the maximum payload size
	/// \param maxPayload the payload size to set as the maximum
	void maxPayload(uint16_t maxPayload);

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

	/// \brief Get the number of insertions
	/// \returns the number of insertions
	uint16_t numInsertions() const;

	/// \brief Set the number of insertions
	/// \param numInsertions the number of insertions to set
	void numInsertions(uint16_t numInsertions);

	/// \brief Get the ZlibCompressedHBF bitset
	/// \returns the ZlibCompressedHBF bitset
	const ZlibCompressedData <std::bitset <HBFSize> > & compressedHBF() const;

	/// \brief Get the start time
	/// \returns the start time
	TimeStamp time() const;

	/// \brief Get the size of the CompressedHBF
	/// \returns the size of the CompressedHBF
	size_t size() const;

	/// \brief Set the size of the CompressedHBF
	/// \param _size the size to set
	void size(size_t _size);

	char * data() {
		return reinterpret_cast<char *>(this);
	}

  private:
	/// The ZlibCompressedHBF version number
	uint8_t _version;

	/// The 13 byte flow ID
	char _flowID[13];

	/// The size of the largest payload
	uint16_t _maxPayload;
	
	/// The start time
	TimeStamp _startTime;

	/// The end time
	TimeStamp _endTime;

	/// The number of insertions
	uint16_t _numInsertions;

	/// The ZlibCompressed HBF bitset
	ZlibCompressedData <std::bitset <HBFSize> > _compressedHBF;
} __attribute__ ((packed));

inline size_t ZlibCompressedHBF::size() const {
	return sizeof(_version) +
		   sizeof(_flowID) +
		   sizeof(_maxPayload) +
		   sizeof(_startTime) +
		   sizeof(_endTime) +
		   sizeof(_numInsertions) +
		   _compressedHBF.size();
}

inline void ZlibCompressedHBF::size(size_t _size) {
	_compressedHBF.size(_size -
							(sizeof(_version) +
							 sizeof(_flowID) +
							 sizeof(_maxPayload) +
							 sizeof(_startTime) +
							 sizeof(_endTime) +
							 sizeof(_numInsertions)));
}

inline uint8_t ZlibCompressedHBF::version() const {
	return _version;
}

inline void ZlibCompressedHBF::version(uint8_t version) {
	_version = version;
}

inline uint8_t ZlibCompressedHBF::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void ZlibCompressedHBF::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t ZlibCompressedHBF::sourceIP() const {
	return ntohl(rawSourceIP());
}

inline void ZlibCompressedHBF::sourceIP(uint32_t sourceIP) {
	rawSourceIP(htonl(sourceIP));
}

inline uint32_t ZlibCompressedHBF::rawSourceIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void ZlibCompressedHBF::rawSourceIP(uint32_t sourceIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= sourceIP;
}

inline uint32_t ZlibCompressedHBF::destinationIP() const {
	return ntohl(rawDestinationIP());
}

inline void ZlibCompressedHBF::destinationIP(uint32_t destinationIP) {
	rawDestinationIP(htonl(destinationIP));
}

inline uint32_t ZlibCompressedHBF::rawDestinationIP() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())));
}

inline void ZlibCompressedHBF::rawDestinationIP(uint32_t destinationIP) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())))
		= destinationIP;
}

inline uint16_t ZlibCompressedHBF::sourcePort() const {
	return ntohs(rawSourcePort());
}

inline void ZlibCompressedHBF::sourcePort(uint16_t sourcePort) {
	rawSourcePort(htons(sourcePort));
}

inline uint16_t ZlibCompressedHBF::rawSourcePort() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())));
}

inline void ZlibCompressedHBF::rawSourcePort(uint16_t sourcePort) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())))
		= sourcePort;
}

inline uint16_t ZlibCompressedHBF::destinationPort() const {
	return ntohs(rawDestinationPort());
}

inline void ZlibCompressedHBF::destinationPort(uint16_t destinationPort) {
	rawDestinationPort(htons(destinationPort));
}

inline uint16_t ZlibCompressedHBF::rawDestinationPort() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())));
}

inline void ZlibCompressedHBF::rawDestinationPort(uint16_t destinationPort) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(sourceIP())
										 + sizeof(destinationIP())
										 + sizeof(sourcePort())))
		= destinationPort;
}

inline uint16_t ZlibCompressedHBF::maxPayload() const {
	return _maxPayload;
}

inline void ZlibCompressedHBF::maxPayload(uint16_t maxPayload) {
	_maxPayload = maxPayload;
}

inline TimeStamp ZlibCompressedHBF::startTime() const {
	return _startTime;
}

inline void ZlibCompressedHBF::startTime(const TimeStamp &startTime) {
	_startTime = startTime;
}

inline TimeStamp ZlibCompressedHBF::endTime() const {
	return _endTime;
}

inline void ZlibCompressedHBF::endTime(const TimeStamp &endTime) {
	_endTime = endTime;
}

inline const ZlibCompressedData <std::bitset <ZlibCompressedHBF::HBFSize> > & 
ZlibCompressedHBF::compressedHBF() const 
{
	return _compressedHBF;
}

inline uint16_t ZlibCompressedHBF::numInsertions() const {
	return _numInsertions;
}

inline void ZlibCompressedHBF::numInsertions(uint16_t numInsertions) {
	_numInsertions = numInsertions;
}

inline TimeStamp ZlibCompressedHBF::time() const {
	return _startTime;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
