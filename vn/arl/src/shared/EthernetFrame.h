#ifndef ETHERNETFRAME_H 
#define ETHERNETFRAME_H

#include <pcap.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include "Frame.h"
#include "FrameTypeTraits.hpp"
#include "DatagramTypeTraits.hpp"
#include "TimeStamp.h"
#include "EthernetAddress.h"

namespace vn {
namespace arl {
namespace shared {

class Frame;

/// \brief An ethernet frame
///
/// This class represents an ethernet frame.
class EthernetFrame {
  public:
	typedef ethernet_frame_type_tag frame_type;

	/// \brief Constructor
	EthernetFrame();
	
	/// \brief Construct from a Frame
	EthernetFrame(const Frame &frame);

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

	/// \brief Get the type of frame this is
	/// \returns the type of frame this is
	FrameType frameType() const;

	/// \brief Get the size of the Ethernet frame header
	/// \returns the size of the Ethernet frame header
	uint16_t frameHeaderSize() const;

	/// \brief Get the size of the payload
	/// \returns the size of the payload
	uint16_t payloadSize() const;

	/// \brief Get the source Ethernet Address
	/// \returns the source Ethernet Address
	EthernetAddress sourceEthernetAddress() const;

	/// \brief Get the destination Ethernet Address
	/// \returns the destination Ethernet Address
	EthernetAddress destinationEthernetAddress() const;

	/// \brief Get the ethertype value from the Ethernet Frame
	/// \returns the ethertype value from the Ethernet Frame
	uint16_t ethernetType() const;

	DatagramType datagramType() const;

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
