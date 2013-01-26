#include "HBF.h"

HBF::HBF()
	:_version(HBF::Version),
	 _maxPayload(0),
	 _startTime(),
	 _endTime(),
	 _hbf(),
	 _numInsertions(0)
{
}

HBF::HBF(const char *flowID, uint16_t payloadSize, const TimeStamp &t)
	:_version(HBF::Version),
	 _maxPayload(payloadSize),
	 _startTime(t),
	 _endTime(t),
	 _hbf(),
	 _numInsertions(0)
{
	memcpy(_flowID, flowID, sizeof(_flowID));
}

int HBF::init(const ZlibCompressedHBF &compressedHBF)
{
	_version = compressedHBF.version();
	_maxPayload = compressedHBF.maxPayload();
	_startTime = compressedHBF.startTime();
	_endTime = compressedHBF.endTime();
	_numInsertions = compressedHBF.numInsertions();
	protocol(compressedHBF.protocol());
	rawSourceIP(compressedHBF.rawSourceIP());
	rawDestinationIP(compressedHBF.rawDestinationIP());
	rawSourcePort(compressedHBF.rawSourcePort());
	rawDestinationPort(compressedHBF.rawDestinationPort());
	return compressedHBF.compressedHBF().uncompress(_hbf);
}
