#ifndef FRAME_H 
#define FRAME_H

#include <pcap.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include "FrameTypeTraits.hpp"
#include "TimeStamp.h"

namespace vn {
namespace arl {
namespace shared {

class EthernetFrame;

/// \brief A Frame
///
/// This class represents a Frame. Any kind of frame. Specific types of
/// frames can construct from this by copying the shared_ptr and shared_array
/// data members and provide an interface for a user to obtain type-specific
/// information.
class Frame {
  public:
	typedef frame_type_tag frame_type;

	/// \brief Constructor
	Frame();

	/// \brief Construct from any frame type
	Frame(const EthernetFrame &frame);

	/// \brief Get the size of the serialized frame data
	/// \returns the size of the serialized frame data
	///
	/// The size of serialized frame data is the size of the header plus the
	/// size of the captured data.
	size_t size() const;

	/// \brief Assign a new value to this Frame
	/// \param pcapHeader a pointer to the pcap_pkthdr to assign
	/// \param pcapData a pointer to the pcap data to assign
	/// \returns true if the new value was successfully assigned
	///
	/// Copies the data pointed to by pcapHeader and pcapData into this Frame
	bool assign(const pcap_pkthdr *pcapHeader, const u_char *pcapData);
	
	/// \brief Get the frame time
	/// \returns the time from the pcap header
	TimeStamp time() const;

	/// \brief Get the frame size
	/// \returns the len value from the pcap header
	uint16_t frameSize() const;

	/// \brief Get the captured size
	/// \returns the caplen value from the pcap header
	uint16_t capturedSize() const;

	/// \brief Get a pointer to the pcap data
	/// \returns a pointer to the pcap data
	const u_char * const payload() const;

	/// \brief Get the _pcapHeader shared_ptr
	/// \returns a copy of the _pcapHeader shared_ptr
	boost::shared_ptr<pcap_pkthdr> pcapHeader() const;

	/// \brief Get the _pcapData shared_array
	/// \returns a copy of the _pcapData shared_array
	boost::shared_array<u_char> pcapData() const;

  private:
	/// The pcap header
  	boost::shared_ptr<pcap_pkthdr> _pcapHeader;

	/// The pcap frame
	boost::shared_array<u_char> _pcapData;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
