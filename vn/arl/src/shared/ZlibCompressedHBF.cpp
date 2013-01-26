#include "ZlibCompressedHBF.h"

namespace vn {
namespace arl {
namespace shared {

const size_t ZlibCompressedHBF::DataSize(sizeof(uint8_t) +
										 13 +
										 sizeof(uint16_t) +
										 sizeof(TimeStamp) +
										 sizeof(TimeStamp) +
										 sizeof(uint16_t) +
										 ZlibCompressedData
											<std::bitset <HBFSize> >::DataSize);

ZlibCompressedHBF::ZlibCompressedHBF()
	:_version(ZlibCompressedHBF::Version),
	 _maxPayload(0),
	 _startTime(),
	 _endTime(),
	 _numInsertions(0),
	 _compressedHBF()
{
}

ZlibCompressedHBF::ZlibCompressedHBF(const char *flowID,
									 uint16_t payloadSize,
									 const TimeStamp &t)
	:_version(ZlibCompressedHBF::Version),
	 _maxPayload(payloadSize),
	 _startTime(t),
	 _endTime(t),
	 _numInsertions(0),
	 _compressedHBF()
{
	memcpy(_flowID, flowID, sizeof(_flowID));
}

ZlibCompressedHBF::ZlibCompressedHBF(const HBF &hbf)
	:_version(ZlibCompressedHBF::Version),
	 _maxPayload(hbf.maxPayload()),
	 _startTime(hbf.startTime()),
	 _endTime(hbf.endTime()),
	 _numInsertions(hbf.numInsertions()),
	 _compressedHBF()
{
	protocol(hbf.protocol());
	rawSourceIP(hbf.rawSourceIP());
	rawDestinationIP(hbf.rawDestinationIP());
	rawSourcePort(hbf.rawSourcePort());
	rawDestinationPort(hbf.rawDestinationPort());
	_compressedHBF.compress(hbf.hbf());
}

} // namespace shared
} // namespace arl
} // namespace vn
