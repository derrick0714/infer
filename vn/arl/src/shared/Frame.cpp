#include <algorithm>

#include "Frame.h"
#include "EthernetFrame.h"

namespace vn {
namespace arl {
namespace shared {

Frame::Frame()
	:_pcapHeader(),
	 _pcapData()
{
}

Frame::Frame(const EthernetFrame &frame)
	:_pcapHeader(frame.pcapHeader()),
	 _pcapData(frame.pcapData())
{
}

size_t Frame::size() const {
	return sizeof(pcap_pkthdr) + _pcapHeader->caplen;
}

bool Frame::assign(const pcap_pkthdr *pcapHeader, const u_char *pcapData) {
	_pcapHeader.reset(new pcap_pkthdr(*pcapHeader));
	_pcapData.reset(new u_char[pcapHeader->caplen]);

	std::copy(pcapData, pcapData + pcapHeader->caplen, _pcapData.get());

	return true;
}

TimeStamp Frame::time() const {
	return TimeStamp(_pcapHeader->ts);
}

uint16_t Frame::frameSize() const {
	return _pcapHeader->len;
}

uint16_t Frame::capturedSize() const {
	return _pcapHeader->caplen;
}

const u_char * const Frame::payload() const {
	return _pcapData.get();
}

boost::shared_ptr <pcap_pkthdr> Frame::pcapHeader() const {
	return _pcapHeader;
}

boost::shared_array <u_char> Frame::pcapData() const {
	return _pcapData;
}

} // namespace shared
} // namespace arl
} // namespace vn
