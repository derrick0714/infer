#ifndef IPV6PACKET_HPP
#define IPV6PACKET_HPP

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <boost/asio/ip/address_v6.hpp>

#include "Packet.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief An IPv6 Packet
///
/// This class represents an IPv6 Packet. There is stuff left TODO. Much
/// of what is TODO requires parsing the IPv6 Extention Headers. With IPv4,
/// for example, getting header fields is straightforward. There is some info
/// in IPv6, however, that is only accessable by fully parsing all of the
/// Extention Headers. In IPv4, we can just return the data on the fly. If
/// we do that here, we'd be parsing the headers each time. This class should
/// be implemented such that Extention Header parsing happens just once, so
/// as to avoid the cost of doing so repeatedly.
template <typename FrameType>
class IPv6Packet : public Packet <FrameType> {
  public:
	/// \brief Virtual destructor
	virtual ~IPv6Packet();

	/// \brief Get the traffic class
	/// \returns the value of the Traffic Class field in the IPv6 header
	///
	/// Analogous to the IPv4 Type of Service field.
	/// TODO
	uint8_t trafficClass() const;

	/// \brief Determine whether or not this packet is fragmented
	/// \returns whether or not this packet is fragmented
	///
	/// TODO
	bool fragmented() const;

	/// \brief Get the Hop Limit
	/// \returns the value of the Hop Limit field from the IPv6 header
	///
	/// Analogous to the IPv4 Time to Live field
	/// TODO
	uint8_t hopLimit() const;

	/// \brief Get the protocol
	/// \returns the value of the Next Header field of the last Extention
	/// Header
	/// TODO
	uint8_t protocol() const;

	/// \brief Get the protocol from the frame pointed to by pcapData
	/// \returns the value of the Next Header field of the last Extention
	/// Header in the IPv6 header contained in the frame pointed to by
	/// pcapData
	/// TODO
	static uint8_t protocol(const u_char *pcapData);

	/// \brief Get the source IPv6 Address
	/// \returns the source IPv6 Address
	boost::asio::ip::address_v6 sourceIP() const;
	
	/// \brief Get the destination IPv6 Address
	/// \returns the destination IPv6 Address
	boost::asio::ip::address_v6 destinationIP() const;

	/// \brief Get the type of packet this is
	/// \returns Packet <FrameType>::TYPE_IPV6
	typename Packet <FrameType>::PacketType packetType() const;

	/// \brief Get the size of the packet header
	/// \returns the size of the IPv6 header
	///
	/// TODO
	uint16_t packetHeaderSize() const;

        bool frame(const FrameType &frame);

        const FrameType& frame() const {
            return _frame;
        }

        SegmentType segmentType() const;

        /// \brief Get the size of the packet header
	/// \returns the size of the IPv4 header
	uint16_t datagramHeaderSize() const {
            return sizeof(ip6_hdr);
        }

        virtual uint16_t payloadSize() const;

  private:
	/// The contained Frame
	FrameType _frame;
};

template <typename FrameType>
uint16_t IPv6Packet <FrameType>::payloadSize() const {
	return _frame.payloadSize() - datagramHeaderSize();
}

template <typename FrameType>
bool IPv6Packet <FrameType>::frame(const FrameType& frame) {
	if (frame.datagramType() != DATAGRAM_IPV6) {
		return false;
	}

	_frame = frame;
	return true;
}

template <typename FrameType>
SegmentType IPv6Packet<FrameType>::segmentType() const {
	switch (protocol()) {
	  case IPPROTO_TCP:
		return SEGMENT_TCP;

	  case IPPROTO_UDP:
		return SEGMENT_UDP;
	}

	return SEGMENT_UNKNOWN;
}


template <typename FrameType>
IPv6Packet <FrameType>::~IPv6Packet() {
}

// TODO
template <typename FrameType>
uint8_t IPv6Packet <FrameType>::trafficClass() const {
	return -1;
}

// TODO
template <typename FrameType>
bool IPv6Packet <FrameType>::fragmented() const {
	return false;
}

//TODO
template <typename FrameType>
uint8_t IPv6Packet <FrameType>::hopLimit() const {
	return -1;
}

// TODO
template <typename FrameType>
uint8_t IPv6Packet <FrameType>::protocol() const {
	return ((ip6_hdr *) (_frame.pcapData().get() + _frame.frameHeaderSize())) -> ip6_nxt;
}

// TODO
template <typename FrameType>
uint8_t IPv6Packet <FrameType>::protocol(const u_char *) {
	return -1;
}

template <typename FrameType>
boost::asio::ip::address_v6 IPv6Packet <FrameType>::sourceIP() const {
	boost::asio::ip::address_v6::bytes_type address;

        boost::shared_array<u_char> pcap_data = this->pcapData();
        ip6_hdr * header = (ip6_hdr *) (pcap_data.get() + this->frameHeaderSize());
        unsigned char* addr = (unsigned char*) &(header->ip6_src);
        
        for (size_t i = 0; i < sizeof(in6_addr); ++i) {
		address[i] = addr[i];
	}

	return boost::asio::ip::address_v6(address);
}

template <typename FrameType>
boost::asio::ip::address_v6 IPv6Packet <FrameType>::destinationIP() const {
	boost::asio::ip::address_v6::bytes_type address;

        boost::shared_array<u_char> pcap_data = this->pcapData();
        ip6_hdr * header = (ip6_hdr *) (pcap_data.get() + this->frameHeaderSize());
        unsigned char* addr = (unsigned char*) &(header->ip6_dst);

        for (size_t i = 0; i < sizeof(in6_addr); ++i) {
		address[i] = addr[i];
	}

        return boost::asio::ip::address_v6(address);
}

template <typename FrameType>
typename Packet <FrameType>::PacketType
	IPv6Packet <FrameType>::packetType() const
{
	return Packet <FrameType>::TYPE_IPV6;
}

// TODO
template <typename FrameType>
uint16_t IPv6Packet <FrameType>::packetHeaderSize() const {
	return datagramHeaderSize();
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
