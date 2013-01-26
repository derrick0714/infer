#include <boost/numeric/conversion/cast.hpp>
#include <iostream>

#include "HTTPRequest.h"

ErrorStatus HTTPRequest::serialize(std::ostream &dest) const {
	printf("derrick is in the HTTPRequest serialize \r\n");
	dest.write(_flowID, 13);
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(reinterpret_cast<const char *>(&_time), sizeof(TimeStamp));
	if (!dest) {
		return E_FSTREAM;
	}
	uint16_t len;
	len = boost::numeric_cast<uint16_t>(_type.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_type.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_uri.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_uri.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_version.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_version.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_host.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_host.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_user_agent.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_user_agent.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	len = boost::numeric_cast<uint16_t>(_referer.length());
	dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(_referer.data(), len);
	if (!dest) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}

ErrorStatus HTTPRequest::unserialize(std::istream &src) {
	printf("derrick is in the HTTPRequestunserialize \r\n");
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
	if (_type.size() == len) {
		_type.resize(0);
	}
	_type.resize(len);
	src.read(const_cast<char *>(_type.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_uri.size() == len) {
		_uri.resize(0);
	}
	_uri.resize(len);
	src.read(const_cast<char *>(_uri.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

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
	if (_host.size() == len) {
		_host.resize(0);
	}
	_host.resize(len);
	src.read(const_cast<char *>(_host.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_user_agent.size() == len) {
		_user_agent.resize(0);
	}
	_user_agent.resize(len);
	src.read(const_cast<char *>(_user_agent.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	src.read(reinterpret_cast<char *>(&len), sizeof(len));
	if (!src) {
		return E_FSTREAM;
	}
	if (_referer.size() == len) {
		_referer.resize(0);
	}
	_referer.resize(len);
	src.read(const_cast<char *>(_referer.data()), len);
	if (!src) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}
