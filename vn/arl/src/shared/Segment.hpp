#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include "SegmentTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A Segment
///
/// This class represents a Segment. Any kind of segment. Specific types of
/// segments shall inherit from this and provide an interface for a user to
/// obtain type-specific information.
template <typename DatagramType>
class Segment {
  public:
	typedef segment_type_tag segment_type;

	/// \brief Get a reference to the contained Datagram
	/// \returns a const reference to the contained Datagram
	const DatagramType & datagram() const;

	/// \brief Assign the contained Datagram
	/// \param datagram the datagram to assign
	/// \returns true if the assignment was successful, false if datagram is
	/// not a UDPSegment
	bool datagram(const DatagramType &datagram);

	/// \brief Get the type of segment this is
	/// \returns TYPE_UNKNOWN
	SegmentType segmentType() const;

  private:
	DatagramType _datagram;
};

template <typename DatagramType>
const DatagramType & Segment <DatagramType>::datagram() const {
	return _datagram;
}

template <typename DatagramType>
bool Segment <DatagramType>::datagram(const DatagramType &datagram) {
	_datagram = datagram;
	return true;
}

template <typename DatagramType>
SegmentType Segment <DatagramType>::segmentType() const
{
	return SEGMENT_UNKNOWN;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
