#ifndef INFER_INCLUDE_FLATFILEHEADERV00_HPP_
#define INFER_INCLUDE_FLATFILEHEADERV00_HPP_

#include <sys/endian.h>

#include "ErrorStatus.hpp"

class FlatFileHeaderV00 {
  public:
	FlatFileHeaderV00()
		:_magicNumber(MagicNumber),
		 _majorVersion(MajorVersion),
		 _minorVersion(MinorVersion),
		 _type(0),
		 _headerFlags(0),
		 _recordCount(0),
		 _recordSize(0)
	{
	}

	uint32_t magicNumber() const {
		return _magicNumber;
	}

	uint8_t majorVersion() const {
		return _majorVersion;
	}

	uint8_t minorVersion() const {
		return _minorVersion;
	}

	uint8_t type() const {
		return _type;
	}

	uint8_t headerFlags() const {
		return _headerFlags;
	}

	uint64_t recordCount() const {
		return _recordCount;
	}

	uint64_t recordSize() const {
		return _recordSize;
	}

	uint64_t indexOffset() const {
		return _recordSize;
	}

	void magicNumber(uint32_t magicNumber) {
		_magicNumber = magicNumber;
	}

	void majorVersion(uint8_t majorVersion) {
		_majorVersion = majorVersion;
	}

	void minorVersion(uint8_t minorVersion) {
		_minorVersion = minorVersion;
	}

	void type(uint8_t type) {
		_type = type;
	}

	void headerFlags(uint8_t headerFlags) {
		_headerFlags = headerFlags;
	}

	void recordCount(uint64_t recordCount) {
		_recordCount = recordCount;
	}

	void recordSize(uint64_t recordSize) {
		_recordSize = recordSize;
	}

	void indexOffset(uint64_t indexOffset) {
		_recordSize = indexOffset;
	}

	ErrorStatus serialize(std::ostream &dest) const {
		dest.write(reinterpret_cast<const char *>(&_magicNumber),
				   sizeof(_magicNumber));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_majorVersion),
				   sizeof(_majorVersion));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_minorVersion),
				   sizeof(_minorVersion));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_type),
				   sizeof(_type));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_headerFlags),
				   sizeof(_headerFlags));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_recordCount),
				   sizeof(_recordCount));
		if (!dest) {
			return E_FSTREAM;
		}
		dest.write(reinterpret_cast<const char *>(&_recordSize),
				   sizeof(_recordSize));
		if (!dest) {
			return E_FSTREAM;
		}

		return E_SUCCESS;
	}

	ErrorStatus unserialize(std::istream &src) {
		bool swap(false);
		
		src.read(reinterpret_cast<char *>(&_magicNumber), sizeof(_magicNumber));
		if (!src) {
			return E_FSTREAM;
		}
		if (_magicNumber != MagicNumber) {
			if (_magicNumber != bswap32(MagicNumber)) {
				return E_BADHEADER;
			}
			swap = true;
		}

		src.read(reinterpret_cast<char *>(&_majorVersion),
				 sizeof(_majorVersion));
		if (!src) {
			return E_FSTREAM;
		}
		if (_majorVersion != MajorVersion) {
			return E_BADVERSION;
		}

		src.read(reinterpret_cast<char *>(&_minorVersion),
				 sizeof(_minorVersion));
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&_type),
				 sizeof(_type));
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&_headerFlags),
				 sizeof(_headerFlags));
		if (!src) {
			return E_FSTREAM;
		}

		src.read(reinterpret_cast<char *>(&_recordCount),
				 sizeof(_recordCount));
		if (!src) {
			return E_FSTREAM;
		}
		if (swap) {
			_recordCount = bswap64(_recordCount);
		}

		src.read(reinterpret_cast<char *>(&_recordSize),
				 sizeof(_recordSize));
		if (!src) {
			return E_FSTREAM;
		}
		if (swap) {
			_recordSize = bswap64(_recordSize);
		}

		return E_SUCCESS;
	}

	void clear() {
		_magicNumber = MagicNumber;
		_majorVersion = MajorVersion;
		_minorVersion = MinorVersion;
		_type = 0;
		_headerFlags = 0;
		_recordCount = 0;
		_recordSize = 0;
	}

  private:
	static const uint32_t MagicNumber = 0x1aad8dd4;
	static const uint8_t MajorVersion = 0x00;
	static const uint8_t MinorVersion = 0x01;

	uint32_t _magicNumber;
	uint8_t _majorVersion;
	uint8_t _minorVersion;
	uint8_t _type;
	uint8_t _headerFlags;
	uint64_t _recordCount;
	uint64_t _recordSize; // index offset if variable-size data
};

#endif
