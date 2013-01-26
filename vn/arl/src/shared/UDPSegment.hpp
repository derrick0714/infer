#ifndef UDPSEGMENT_HPP
#define UDPSEGMENT_HPP

#include <stdint.h>

#include "SegmentTypeTraits.hpp"
#include "Segment.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A UDP Segment
///
/// This class represents a UDP Segment.
template <typename DatagramType>
class UDPSegment {
  public:
	typedef udp_segment_type_tag segment_type;

	/// \brief Constructor
	UDPSegment();

	/// \brief Get a reference to the contained Datagram
	/// \returns a const reference to the contained Datagram
	const DatagramType & datagram() const;

	/// \brief Assign the contained Datagram
	/// \param datagram the datagram to assign
	/// \returns true if the assignment was successful, false if datagram is
	/// not a UDPSegment
	bool datagram(const DatagramType &datagram);

	/// \brief Get the source port
	/// \returns the value of the source port field in the UDP header
	uint16_t sourcePort();

	/// \brief Get the destination port
	/// \returns the value of the destination port field in the UDP header
	uint16_t destinationPort();

	/// \brief Get the source port in network byte order
	/// \returns the value of the source port field in the UDP header in
	/// network byte order
	uint16_t rawSourcePort();

	/// \brief Get the destination port in network byte order
	/// \returns the value of the destination port field in the UDP header in
	/// network byte order
	uint16_t rawDestinationPort();

	/// \brief Get the UDP length
	/// \returns the value of the Length field in the UDP header
	uint16_t udpLength();

	/// \brief Get the UDP checksum
	/// \returns the value of the UDP checksum field in the UDP header
	///
	/// Note: The checksum is not validated.
	uint16_t udpChecksum();

	/// \brief Get the type of segment this is
	/// \returns SEGMENT_UDP
	SegmentType segmentType() const;

	/// \brief Get the size of the segment header
	/// \returns the size of the UDP header
	uint16_t segmentHeaderSize() const;

	uint16_t payloadSize() const;

	const u_char * payload() const;

  private:
	/// The contained Datagram
	DatagramType _datagram;

	struct udphdr {
		u_short uh_sport;		/* source port */
		u_short uh_dport;		/* destination port */
		u_short uh_ulen;		/* udp length */
		u_short uh_sum;			/* udp checksum */
	};
};

template <typename DatagramType>
UDPSegment <DatagramType>::UDPSegment()
	:_datagram()
{
}

template <typename DatagramType>
const DatagramType & UDPSegment <DatagramType>::datagram() const {
	return _datagram;
}

template <typename DatagramType>
bool UDPSegment <DatagramType>::datagram(const DatagramType &datagram) {
	if (datagram.segmentType() != SEGMENT_UDP) {
		return false;
	}

	_datagram = datagram;
	return true;
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::sourcePort() {
	return ntohs(
			((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_sport);
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::destinationPort() {
	return ntohs(
			((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_dport);
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::rawSourcePort() {
	return ((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_sport;
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::rawDestinationPort() {
	return ((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_dport;
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::udpLength() {
	return ntohs(
			((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_ulen);
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::udpChecksum() {
	return ntohs(
			((udphdr *) (_datagram.frame().pcapData().get() + _datagram.frame().frameHeaderSize() + _datagram.datagramHeaderSize())) -> uh_sum);
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::segmentHeaderSize() const {
	return sizeof(udphdr);
}

template <typename DatagramType>
uint16_t UDPSegment <DatagramType>::payloadSize() const {
	return _datagram.payloadSize() - segmentHeaderSize();
}

template <typename DatagramType>
SegmentType UDPSegment <DatagramType>::segmentType() const
{
	return SEGMENT_UDP;
}

template <typename _DatagramType>
const u_char * UDPSegment <_DatagramType>::payload() const {
	return _datagram.frame().pcapData().get() +
				(_datagram.frame().capturedSize() - payloadSize());
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
