#ifndef INFER_INCLUDE_HBF_HPP_
#define INFER_INCLUDE_HBF_HPP_

// NB: this file deprecates hbf.h and hbf.cpp

#include <bitset>
#include <netinet/in.h>

#include "DataTypeTraits.hpp"
#include "timeStamp.h"
#include "ZlibCompressedHBF.h"

class ZlibCompressedHBF;

class hbf {
  public:
	typedef plain_old_data_tag data_type;

	enum {
		hbf_version = 3,     /// The hbf Version Number
		hbf_size = 32768, /// The size of the hbf bitset
		block_size = 64,  /// The block size
		num_hashes = 2    /// The number of hashes
	};

	/// \brief Default constructor
	hbf();

	/// \brief Construct with time and payload size
	/// \param flowID a pointer to the 13 byte flow ID
	/// \param payloadSize the first payloadSize
	/// \param t the first time
	///
	/// Constructs a new hbf, setting the flow ID to flowID, start and end times
	/// to t, and the size of the largest payload to payloadSize
	hbf(const char *flowID, uint16_t payloadSize, const TimeStamp &t);

	/// \brief Init from a ZlibCompressedHBF
	/// \param the ZlibCompressedHBF to init from
	int init(const ZlibCompressedHBF &compressedHBF);

	/// \brief Get the hbf version number
	/// \returns the hbf version number
	uint8_t version() const;

	/// \brief Set the hbf version number
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

	/// \brief Get the maximum payload size
	/// \returns the maximum payload size
	uint16_t max_payload() const;

	/// \brief Set the maximum payload size
	/// \param max_payload the payload size to set as the maximum
	void max_payload(uint16_t max_payload);

	/// \brief Get the start time
	/// \returns the start time
	TimeStamp start_time() const;

	/// \brief Set the start time
	/// \param start_time the start time to set
	void start_time(const TimeStamp &start_time);

	/// \brief Get the end time
	/// \returns the end time
	TimeStamp end_time() const;

	/// \brief Set the end time
	/// \param end_time the end time to set
	void end_time(const TimeStamp &end_time);

	bool test(uint32_t n) const;

	/// \brief Get the number of insertions
	/// \returns the number of insertions
	uint16_t num_insertions() const;

	/// \brief Set the number of insertions
	/// \param num_insertions the number of insertions to set
	void num_insertions(uint16_t num_insertions);

	/// \brief Set a bit in the hbf
	/// \param bit the bit to set
	///
	/// Sets bit bit in _hbf and increments _num_insertions
	void setBit(uint32_t bit);

	/// \brief Get the start time
	/// \returns the start time
	TimeStamp time() const;

  private:
	/// The hbf version number
	uint8_t _version;

	/// The 13 byte flow ID
	char _flowID[13];

	/// The size of the largest payload
	uint16_t _max_payload;
	
	/// The start time
	TimeStamp _start_time;

	/// The end time
	TimeStamp _end_time;

	/// The hbf bitset
	std::bitset <hbf_size> _hbf;

	/// The number of insertions
	uint16_t _num_insertions;

	/// Padding
	char _padding[6];
};

inline uint8_t hbf::version() const {
	return _version;
}

inline void hbf::version(uint8_t version) {
	_version = version;
}

inline uint8_t hbf::protocol() const {
	return *(reinterpret_cast<const uint8_t *>(_flowID));
}

inline void hbf::protocol(uint8_t protocol) {
	*(reinterpret_cast<uint8_t *>(_flowID))
		= protocol;
}

inline uint32_t hbf::source_ip() const {
	return ntohl(raw_source_ip());
}

inline void hbf::source_ip(uint32_t source_ip) {
	raw_source_ip(htonl(source_ip));
}

inline uint32_t hbf::raw_source_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())));
}

inline void hbf::raw_source_ip(uint32_t source_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())))
		= source_ip;
}

inline uint32_t hbf::destination_ip() const {
	return ntohl(raw_destination_ip());
}

inline void hbf::destination_ip(uint32_t destination_ip) {
	raw_destination_ip(htonl(destination_ip));
}

inline uint32_t hbf::raw_destination_ip() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())));
}

inline void hbf::raw_destination_ip(uint32_t destination_ip) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())))
		= destination_ip;
}

inline uint16_t hbf::source_port() const {
	return ntohs(raw_source_port());
}

inline void hbf::source_port(uint16_t source_port) {
	raw_source_port(htons(source_port));
}

inline uint16_t hbf::raw_source_port() const {
	return *(reinterpret_cast<const uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())));
}

inline void hbf::raw_source_port(uint16_t source_port) {
	*(reinterpret_cast<uint32_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())))
		= source_port;
}

inline uint16_t hbf::destination_port() const {
	return ntohs(raw_destination_port());
}

inline void hbf::destination_port(uint16_t destination_port) {
	raw_destination_port(htons(destination_port));
}

inline uint16_t hbf::raw_destination_port() const {
	return *(reinterpret_cast<const uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())));
}

inline void hbf::raw_destination_port(uint16_t destination_port) {
	*(reinterpret_cast<uint16_t *>
								(_flowID + sizeof(protocol())
										 + sizeof(source_ip())
										 + sizeof(destination_ip())
										 + sizeof(source_port())))
		= destination_port;
}

inline uint16_t hbf::max_payload() const {
	return _max_payload;
}

inline void hbf::max_payload(uint16_t max_payload) {
	_max_payload = max_payload;
}

inline TimeStamp hbf::start_time() const {
	return _start_time;
}

inline void hbf::start_time(const TimeStamp &start_time) {
	_start_time = start_time;
}

inline TimeStamp hbf::end_time() const {
	return _end_time;
}

inline void hbf::end_time(const TimeStamp &end_time) {
	_end_time = end_time;
}

inline bool hbf::test(uint32_t n) const {
	return _hbf.test(n);
}

inline uint16_t hbf::num_insertions() const {
	return _num_insertions;
}

inline void hbf::num_insertions(uint16_t num_insertions) {
	_num_insertions = num_insertions;
}

inline void hbf::setBit(uint32_t bit) {
	_hbf.set(bit);
	++_num_insertions;
}

inline TimeStamp hbf::time() const {
	return _start_time;
}

inline hbf::hbf()
	:_version(hbf::hbf_version),
	 _max_payload(0),
	 _start_time(),
	 _end_time(),
	 _hbf(),
	 _num_insertions(0)
{
}

inline hbf::hbf(const char *flowID, uint16_t payloadSize, const TimeStamp &t)
	:_version(hbf::hbf_version),
	 _max_payload(payloadSize),
	 _start_time(t),
	 _end_time(t),
	 _hbf(),
	 _num_insertions(0)
{
	memcpy(_flowID, flowID, sizeof(_flowID));
}

inline int hbf::init(const ZlibCompressedHBF &compressedHBF)
{
	_version = compressedHBF.version();
	_max_payload = compressedHBF.maxPayload();
	_start_time = compressedHBF.startTime();
	_end_time = compressedHBF.endTime();
	_num_insertions = compressedHBF.numInsertions();
	protocol(compressedHBF.protocol());
	raw_source_ip(compressedHBF.rawSourceIP());
	raw_destination_ip(compressedHBF.rawDestinationIP());
	raw_source_port(compressedHBF.rawSourcePort());
	raw_destination_port(compressedHBF.rawDestinationPort());
	return compressedHBF.compressedHBF().uncompress(_hbf);
}

#endif
