#include <algorithm>
#include <netinet/in.h>

#include "EthernetFrame.h"

namespace vn {
namespace arl {
namespace shared {

EthernetFrame::EthernetFrame()
	:_pcapHeader(),
	 _pcapData()
{
}

EthernetFrame::EthernetFrame(const Frame &frame)
	:_pcapHeader(frame.pcapHeader()),
	 _pcapData(frame.pcapData())
{
}

size_t EthernetFrame::size() const {
	return sizeof(pcap_pkthdr) + _pcapHeader->caplen;
}

bool EthernetFrame::assign(const pcap_pkthdr *pcapHeader, const u_char *pcapData) {
	_pcapHeader.reset(new pcap_pkthdr(*pcapHeader));
	_pcapData.reset(new u_char[pcapHeader->caplen]);

	std::copy(pcapData, pcapData + pcapHeader->caplen, _pcapData.get());

	return true;
}

TimeStamp EthernetFrame::time() const {
	return TimeStamp(_pcapHeader->ts);
}

uint16_t EthernetFrame::frameSize() const {
	return _pcapHeader->len;
}

uint16_t EthernetFrame::capturedSize() const {
	return _pcapHeader->caplen;
}

const u_char * const EthernetFrame::payload() const {
	return _pcapData.get();
}

boost::shared_ptr <pcap_pkthdr> EthernetFrame::pcapHeader() const {
	return _pcapHeader;
}

boost::shared_array <u_char> EthernetFrame::pcapData() const {
	return _pcapData;
}

FrameType EthernetFrame::frameType() const {
	return FRAME_ETHERNET;
}

uint16_t EthernetFrame::frameHeaderSize() const {
	return ETHER_HDR_LEN;
}

uint16_t EthernetFrame::payloadSize() const {
	return _pcapHeader->caplen - ETHER_HDR_LEN;
}

EthernetAddress EthernetFrame::sourceEthernetAddress() const {
	return EthernetAddress (((ether_header *) _pcapData.get()) -> ether_shost);
}

EthernetAddress EthernetFrame::destinationEthernetAddress() const {
	return EthernetAddress(std::string
			((char *) (((ether_header *) _pcapData.get()) -> ether_dhost),
			 ETHER_ADDR_LEN));
}

uint16_t EthernetFrame::ethernetType() const {
	return ntohs(((ether_header *) _pcapData.get()) -> ether_type);
}

// Some systems don't define this.
#ifndef ETHERTYPE_IPV6
#define	ETHERTYPE_IPV6		0x86dd
#endif

DatagramType EthernetFrame::datagramType() const {
	switch (ethernetType()) {
	    case ETHERTYPE_IP:
		return DATAGRAM_IPV4;

            case ETHERTYPE_IPV6:
                return DATAGRAM_IPV6;
	}

	return DATAGRAM_UNKNOWN;
}

} // namespace shared
} // namespace arl
} // namespace vn
