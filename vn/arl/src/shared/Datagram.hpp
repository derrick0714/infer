#ifndef DATAGRAM_HPP
#define DATAGRAM_HPP

#include "DatagramTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A Datagram
///
/// This class represents a Datagram.
template <typename _FrameType>
class Datagram {
  public:
	typedef datagram_type_tag datagram_type;

	Datagram();

	/// \brief Get a reference to the contained Frame
	/// \returns a const reference to the contained Frame
	const _FrameType & frame() const;

	/// \brief Assign the contained Frame
	/// \param frame the frame to assign
	/// \returns true if the assignment was successful, false if frame is not
	/// an Datagram
	bool frame(const _FrameType &frame);

	DatagramType datagramType() const;

  private:
	/// The contained Frame
	_FrameType _frame;
};

template <typename _FrameType>
Datagram <_FrameType>::Datagram()
	:_frame()
{
}

template <typename _FrameType>
const _FrameType & Datagram <_FrameType>::frame() const {
	return _frame;
}

template <typename _FrameType>
bool Datagram <_FrameType>::frame(const _FrameType &frame) {
	_frame = frame;
	return true;
}

template <typename _FrameType>
DatagramType Datagram <_FrameType>::datagramType() const {
	return DATAGRAM_UNKNOWN;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
