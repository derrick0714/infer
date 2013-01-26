#ifndef FLOWSEGMENT_HPP
#define FLOWSEGMENT_HPP

#include "Segment.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename PacketType>
class FlowSegment : public Segment <PacketType> {
  public:
	/// \brief Virtual destructor
	virtual ~FlowSegment();

	/// \brief Get the source port
	/// \returns the value of the source port field in the Segment header
	virtual uint16_t sourcePort() = 0;

	/// \brief Get the destination port
	/// \returns the value of the destination port field in the Segment header
	virtual uint16_t destinationPort() = 0;

	/// \brief Get the source port in network byte order
	/// \returns the value of the source port field in the Segment header in
	/// network byte order
	virtual uint16_t rawSourcePort() = 0;

	/// \brief Get the destination port in network byte order
	/// \returns the value of the destination port field in the Segment header
	/// in network byte order
	virtual uint16_t rawDestinationPort() = 0;
};

template <typename PacketType>
FlowSegment <PacketType>::~FlowSegment() {
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
