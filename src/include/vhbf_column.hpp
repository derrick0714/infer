#ifndef INFER_INCLUDE_VHBF_COLUMN_HPP_
#define INFER_INCLUDE_VHBF_COLUMN_HPP_

#include <cstring>

#include <boost/static_assert.hpp>
#include <boost/array.hpp>

template <size_t Height>
class basic_vhbf_column;

typedef basic_vhbf_column<4 * 1024 * 8> vhbf_column;

template <size_t Height>
class basic_vhbf_column {
  public:
	static const size_t height = Height;

	void reset() {
		std::memset(&_col, 0, sizeof(_col));
	}

	bool test(size_t row) const {
		return (_col[row / 8] >> (row % 8)) & 0x01;
	}

	void set(size_t row) {
		_col[row / 8] |= 0x01 << (row % 8);
	}

  private:
	BOOST_STATIC_ASSERT(height % 8 == 0);

	boost::array<uint8_t, height / 8> _col;
};

#endif
