#ifndef PACKET_HPP
#define PACKET_HPP

#include "Frame.h"

namespace vn {
namespace arl {
namespace shared {

/// \brief A Packet
///
/// This class represents a Packet. Any kind of packet. Specific types of
/// packets shall inherit from this and provide an interface for a user to
/// obtain type-specific information.
template <typename FrameType>
class Packet : public FrameType {
  public:
	typedef enum
				{
					TYPE_IPV4 = 0x0800,
					TYPE_IPV6 = 0x86DD
				} PacketType;

	/// \brief Virtual destructor
	virtual ~Packet();

	/// \brief Get the type of packet this is
	/// \returns the type of packet this is
	virtual PacketType packetType() const = 0;

	/// \brief Get the size of the packet header
	/// \returns the size of the packet header
	virtual uint16_t packetHeaderSize() const = 0;
	
	/// \brief Get the size of the payload
	/// \returns the size of the payload
	virtual uint16_t payloadSize() const;
};

template <typename FrameType>
Packet <FrameType>::~Packet() {
}
	
template <typename FrameType>
uint16_t Packet <FrameType>::payloadSize() const {
	return FrameType::capturedSize() - FrameType::frameHeaderSize() - packetHeaderSize();
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
