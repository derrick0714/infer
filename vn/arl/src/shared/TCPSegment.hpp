#ifndef TCPSEGMENT_HPP
#define TCPSEGMENT_HPP

#include <stdint.h>

#include "SegmentTypeTraits.hpp"
#include "Segment.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A TCP Segment
///
/// This class represents a TCP Segment.
template <typename _DatagramType>
class TCPSegment {
  public:
	typedef tcp_segment_type_tag segment_type;

	/// \brief Constructor
	TCPSegment();

	/// \brief Get a reference to the contained Datagram
	/// \returns a const reference to the contained Datagram
	const _DatagramType & datagram() const;

	/// \brief Assign the contained Datagram
	/// \param datagram the datagram to assign
	/// \returns true if the assignment was successful, false if datagram is
	/// not a UDPSegment
	bool datagram(const _DatagramType &datagram);

	/// \brief Get the source port in host byte order
	/// \returns the value of the source port field in the TCP header
	uint16_t sourcePort();

	/// \brief Get the destination port in host byte order
	/// \returns the value of the destination port field in the TCP header
	uint16_t destinationPort();

	/// \brief Get the source port in network byte order
	/// \returns the value of the source port field in the TCP header in
	/// network byte order
	uint16_t rawSourcePort();

	/// \brief Get the destination port in network byte order
	/// \returns the value of the destination port field in the TCP header in
	/// network byte order
	uint16_t rawDestinationPort();

	/// \brief Get the sequence number
	/// \returns the value of the sequence number field in the TCP header
	uint32_t sequenceNumber();

	/// \brief Get the acknowledgement number
	/// \returns the value of the acknowledgement number field in the TCP
	/// header
	uint32_t acknowledgementNumber();

	/// \brief Determine if the CWR bit is set
	/// \returns true if the CWR bit is set in the TCP header
	bool cwr();

	/// \brief Determine if the ECE bit is set
	/// \returns true if the ECE bit is set in the TCP header
	bool ece();

	/// \brief Determine if the URG bit is set
	/// \returns true if the URG bit is set in the TCP header
	bool urg();
	
	/// \brief Determine if the ACK bit is set
	/// \returns true if the ACK bit is set in the TCP header
	bool ack();
	
	/// \brief Determine if the PSH bit is set
	/// \returns true if the PSH bit is set in the TCP header
	bool psh();
	
	/// \brief Determine if the RST bit is set
	/// \returns true if the RST bit is set in the TCP header
	bool rst();
	
	/// \brief Determine if the SYN bit is set
	/// \returns true if the SYN bit is set in the TCP header
	bool syn();
	
	/// \brief Determine if the FIN bit is set
	/// \returns true if the FIN bit is set in the TCP header
	bool fin();

	/// \brief Get the window size
	/// \returns the value of the Window Size field in the TCP header
	uint16_t windowSize();

	/// \brief Get the TCP checksum
	/// \returns the value of the TCP checksum field in the TCP header
	///
	/// Note: The checksum is not validated.
	uint16_t tcpChecksum();

	/// \brief Get the type of segment this is
	/// \returns SEGMENT_TCP
	SegmentType segmentType() const;

	/// \brief Get the size of the segment header
	/// \returns the size of the TCP header
	uint16_t segmentHeaderSize() const;

	uint16_t payloadSize() const;

	const u_char * payload() const;

  private:
	/// The contained Datagram
	_DatagramType _datagram;

	enum {
		FLAG_FIN  = 0x01,
		FLAG_SYN  = 0x02,
		FLAG_RST  = 0x04,
		FLAG_PUSH = 0x08,
		FLAG_ACK  = 0x10,
		FLAG_URG  = 0x20,
		FLAG_ECE  = 0x40,
		FLAG_CWR  = 0x80
	};
	struct tcphdr {
		uint16_t th_sport;		/* source port */
		uint16_t th_dport;		/* destination port */
		uint32_t th_seq;		/* sequence number */
		uint32_t th_ack;		/* acknowledgement number */
		#if BYTE_ORDER == LITTLE_ENDIAN
			uint32_t th_x2:4,	/* (unused) */
					th_off:4;	/* data offset */
		#endif
		#if BYTE_ORDER == BIG_ENDIAN
			uint32_t th_off:4,	/* data offset */
					th_x2:4;	/* (unused) */
		#endif
		uint8_t  th_flags;

		uint16_t th_win;		/* window */
		uint16_t th_sum;		/* checksum */
		uint16_t th_urp;		/* urgent pointer */
	};
};

template <typename _DatagramType>
TCPSegment <_DatagramType>::TCPSegment()
	:_datagram()
{
}

template <typename _DatagramType>
const _DatagramType & TCPSegment <_DatagramType>::datagram() const {
	return _datagram;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::datagram(const _DatagramType &datagram) {
	if (datagram.segmentType() != SEGMENT_TCP) {
		return false;
	}

	_datagram = datagram;
	return true;
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::sourcePort() {
	return ntohs(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_sport);
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::destinationPort() {
	return ntohs(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_dport);
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::rawSourcePort() {
	return ((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_sport;
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::rawDestinationPort() {
	return ((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_dport;
}

template <typename _DatagramType>
uint32_t TCPSegment <_DatagramType>::sequenceNumber() {
	return ntohl(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_seq);
}

template <typename _DatagramType>
uint32_t TCPSegment <_DatagramType>::acknowledgementNumber() {
	return ntohl(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_ack);
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::cwr() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_CWR;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::ece() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_ECE;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::urg() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_URG;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::ack() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_ACK;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::psh() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_PUSH;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::rst() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_RST;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::syn() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_SYN;
}

template <typename _DatagramType>
bool TCPSegment <_DatagramType>::fin() {
	return (((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_flags) & 
			FLAG_FIN;
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::windowSize() {
	return ntohs(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_win);
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::tcpChecksum() {
	return ntohs(
			((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_sum);
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::segmentHeaderSize() const {
	return ((((tcphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> th_off) << 2);
}

template <typename _DatagramType>
uint16_t TCPSegment <_DatagramType>::payloadSize() const {
	return _datagram.payloadSize() - segmentHeaderSize();
}

template <typename _DatagramType>
SegmentType TCPSegment <_DatagramType>::segmentType() const
{
	return SEGMENT_TCP;
}

template <typename _DatagramType>
const u_char * TCPSegment <_DatagramType>::payload() const {
	return _datagram.frame().pcapData().get() +
				(_datagram.frame().capturedSize() - payloadSize());
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
