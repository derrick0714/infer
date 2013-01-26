#ifndef IPV4DATAGRAM_HPP
#define IPV4DATAGRAM_HPP

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <boost/asio/ip/address_v4.hpp>

#include "DatagramTypeTraits.hpp"
#include "SegmentTypeTraits.hpp"
#include "Segment.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief An IPv4 Datagram
///
/// This class represents an IPv4 Datagram.
template <typename _FrameType>
class IPv4Datagram {
  public:
	typedef ipv4_datagram_type_tag datagram_type;

	IPv4Datagram();

	/// \brief Get a reference to the contained Frame
	/// \returns a const reference to the contained Frame
	const _FrameType & frame() const;

	/// \brief Assign the contained Frame
	/// \param frame the frame to assign
	/// \returns true if the assignment was successful, false if frame is not
	/// an IPv4Datagram
	bool frame(const _FrameType &frame);

	/// \brief Get the type of service
	/// \returns the value of the Type of Service field in the IPv4 header
	uint8_t typeOfService() const;

	/// \brief Determine whether or not this packet is fragmented
	/// \returns true if this packet is fragmented
	///
	/// An IPv4 packet is considered fragmented if the MF (More Fragments) bit
	/// is set, or if it is not set and the Fragment Offset field is not zero.
	bool fragmented() const;

	/// \brief Get the ttl
	/// \returns the value of the Time to Live field in the IPv4 header
	uint8_t ttl() const;

	/// \brief Get the protocol
	/// \returns the value of the Protocol field in the IPv4 header
	uint8_t protocol() const;

	/// \brief Get the protocol from the frame pointed to by pcapData
	/// \returns the value of the Protocol field in the IPv4 header contained
	/// in the frame pointed to by pcapData
	static uint8_t protocol(const u_char *pcapData);

	/// \brief Get the source IPv4 Address
	/// \returns the source IPv4 Address in the IPv4 header
	boost::asio::ip::address_v4 sourceIP() const;
	
	/// \brief Get the destination IPv4 Address
	/// \returns the destination IPv4 Address in the IPv4 header
	boost::asio::ip::address_v4 destinationIP() const;

	/// \brief Get the source IPv4 Address in network byte order
	/// \returns the raw source IPv4 Address in the IPv4 header in network byte
	/// order
	uint32_t rawSourceIP() const;
	
	/// \brief Get the destination IPv4 Address in network byte order
	/// \returns the raw destination IPv4 Address in the IPv4 header in network
	/// byte order
	uint32_t rawDestinationIP() const;

	/// \brief Get the type of packet this is
	/// \returns Packet <_FrameType>::TYPE_IPV4
	DatagramType datagramType() const;

	/// \brief Get the size of the packet header
	/// \returns the size of the IPv4 header
	uint16_t datagramHeaderSize() const;

	uint16_t payloadSize() const;

	SegmentType segmentType() const;

  private:
	/// The contained Frame
	_FrameType _frame;
};

template <typename _FrameType>
IPv4Datagram <_FrameType>::IPv4Datagram()
	:_frame()
{
}

template <typename _FrameType>
const _FrameType & IPv4Datagram <_FrameType>::frame() const {
	return _frame;
}

template <typename _FrameType>
bool IPv4Datagram <_FrameType>::frame(const _FrameType &frame) {
	if (frame.datagramType() != DATAGRAM_IPV4) {
		return false;
	}

	_frame = frame;
	return true;
}

template <typename _FrameType>
uint8_t IPv4Datagram <_FrameType>::typeOfService() const {
	return ((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) 
																	-> ip_tos;
}

template <typename _FrameType>
bool IPv4Datagram <_FrameType>::fragmented() const {
	return (ntohs(((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_off)
			& IP_MF) != 0
			||
		   (ntohs(((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_off)
			& IP_OFFMASK) != 0;
}

template <typename _FrameType>
uint8_t IPv4Datagram <_FrameType>::ttl() const {
	return ((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_ttl;
}

template <typename _FrameType>
uint8_t IPv4Datagram <_FrameType>::protocol() const {
	return ((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_p;
}

template <typename _FrameType>
uint8_t IPv4Datagram <_FrameType>::protocol(const u_char *pcapData) {
	return ((ip *) (pcapData + _FrameType::frameHeaderSize(pcapData))) -> ip_p;
}

template <typename _FrameType>
boost::asio::ip::address_v4 IPv4Datagram <_FrameType>::sourceIP() const {
	return boost::asio::ip::address_v4(ntohl
			(((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_src.s_addr));
}

template <typename _FrameType>
boost::asio::ip::address_v4 IPv4Datagram <_FrameType>::destinationIP() const {
	return boost::asio::ip::address_v4(ntohl
			(((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_dst.s_addr));
}

template <typename _FrameType>
uint32_t IPv4Datagram <_FrameType>::rawSourceIP() const {
	return ((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_src.s_addr;
}

template <typename _FrameType>
uint32_t IPv4Datagram <_FrameType>::rawDestinationIP() const {
	return ((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_dst.s_addr;
}

template <typename _FrameType>
DatagramType IPv4Datagram <_FrameType>::datagramType() const
{
	return DATAGRAM_IPV4;
}

template <typename _FrameType>
uint16_t IPv4Datagram <_FrameType>::datagramHeaderSize() const {
	return ((((ip *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip_hl) << 2);
}

template <typename _FrameType>
uint16_t IPv4Datagram <_FrameType>::payloadSize() const {
	return _frame.payloadSize() - datagramHeaderSize();
}

template <typename _FrameType>
SegmentType IPv4Datagram<_FrameType>::segmentType() const {
	switch (protocol()) {
	  case IPPROTO_TCP:
		return SEGMENT_TCP;

	  case IPPROTO_UDP:
		return SEGMENT_UDP;
	}

	return SEGMENT_UNKNOWN;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
