#include <boost/numeric/conversion/cast.hpp>
#include <iostream>

#include "HTTPResponse.h"

ErrorStatus HTTPResponse::serialize(std::ostream &dest) const {
	dest.write(_flowID, 13);
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(reinterpret_cast<const char *>(&_time), sizeof(TimeStamp));
	if (!dest) {
		return E_FSTREAM;
	}
	uint16_t len;
	len = boost::numeric_cast<uint16_t>(_version.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_version.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_status.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_status.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_reason.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_reason.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_response.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_response.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_content_type.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_content_type.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}

ErrorStatus HTTPResponse::unserialize(std::istream &src) {
	
	src.read(_flowID, 13);
	if (!src) {
		return E_FSTREAM;
	}
	src.read(reinterpret_cast<char *>(&_time), sizeof(TimeStamp));
	if (!src) {
		return E_FSTREAM;
	}

	uint16_t len;
	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_version.size() == len) {
		_version.resize(0);
	}
	_version.resize(len);
	src.read(const_cast<char *>(_version.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_status.size() == len) {
		_status.resize(0);
	}
	_status.resize(len);
	src.read(const_cast<char *>(_status.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_reason.size() == len) {
		_reason.resize(0);
	}
	_reason.resize(len);
	src.read(const_cast<char *>(_reason.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_response.size() == len) {
		_response.resize(0);
	}
	_response.resize(len);
	src.read(const_cast<char *>(_response.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_content_type.size() == len) {
		_content_type.resize(0);
	}
	_content_type.resize(len);
	src.read(const_cast<char *>(_content_type.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}
