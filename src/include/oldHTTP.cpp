#include <boost/numeric/conversion/cast.hpp>
#include <iostream>

#include "oldHTTP.h"

ErrorStatus OldHTTP::serialize(std::ostream &dest) const {
	dest.write(_flowID + 1, 12);
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(reinterpret_cast<const char *>(&_time), sizeof(TimeStamp));
	if (!dest) {
		return E_FSTREAM;
	}
	dest.write(&_type, 1);
	if (!dest) {
		return E_FSTREAM;
	}
	uint16_t len;
	switch (_type) {
	  case 'q':
		len = boost::numeric_cast<uint16_t>(_requestType.length());
		dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(_requestType.data(), len);
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

		len = boost::numeric_cast<uint16_t>(_userAgent.length());
		dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(_userAgent.data(), len);
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
		break;

	  case 's':
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

		len = boost::numeric_cast<uint16_t>(_contentType.length());
		dest.write(reinterpret_cast<const char *>(&len), sizeof(len));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(_contentType.data(), len);
		if (!dest) {
			return E_FSTREAM;
		}
		break;
	}

	return E_SUCCESS;
}

ErrorStatus OldHTTP::unserialize(std::istream &src) {
	clear();
	protocol(6); // all http data is TCP
	src.read(_flowID + 1, 12);
	if (!src) {
		return E_FSTREAM;
	}
	src.read(reinterpret_cast<char *>(&_time), sizeof(TimeStamp));
	if (!src) {
		return E_FSTREAM;
	}
	src.read(&_type, 1);
	if (!src) {
		return E_FSTREAM;
	}

	uint16_t len;
	switch (_type) {
	  case 'q':
		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
		}
		_requestType.resize(len);
		src.read(const_cast<char *>(_requestType.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
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
		_version.resize(len);
		src.read(const_cast<char *>(_version.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
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
		_userAgent.resize(len);
		src.read(const_cast<char *>(_userAgent.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
		}
		_referer.resize(len);
		src.read(const_cast<char *>(_referer.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		break;

	  case 's':
		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
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
		_status.resize(len);
		src.read(const_cast<char *>(_status.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
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
		_response.resize(len);
		src.read(const_cast<char *>(_response.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
		}
		_contentType.resize(len);
		src.read(const_cast<char *>(_contentType.data()), len);
		if (!src) {
			return E_FSTREAM;
		}

		break;
	}

	return E_SUCCESS;
}
