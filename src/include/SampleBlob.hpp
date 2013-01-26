#ifndef INFER_INCLUDE_SAMPLEBLOB_HPP_
#define INFER_INCLUDE_SAMPLEBLOB_HPP_

#include "DataTypeTraits.hpp"

class SampleBlob {
  public:
	typedef blob_data_tag data_type;

	static const uint8_t TypeID = 0x8F;

	SampleBlob()
		:_size(0)
	{
		memset(reinterpret_cast<void *>(_data), 0, sizeof(_data));
	}

	size_t size() const {
		return _size;
	}

	void size(size_t size) {
		_size = size;
	}

	char & operator[](size_t pos) {
		return _data[pos];
	}

	bool operator==(const SampleBlob &rhs) const {
		if (_size != rhs._size) {
			return false;
		}

		for (size_t i(0); i < _size; ++i) {
			if (_data[i] != rhs._data[i]) {
				return false;
			}
		}

		return true;
	}

	bool operator!=(const SampleBlob &rhs) const {
		return !operator==(rhs);
	}

  private:
	char _data[1024];

	size_t _size;
};

#endif
