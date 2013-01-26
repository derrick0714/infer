#ifndef INFER_INCLUDE_VHBF_CHUNK_HPP_
#define INFER_INCLUDE_VHBF_CHUNK_HPP_

#include <cstring>

#include <boost/static_assert.hpp>

#include "vhbf_column.hpp"
#include "vhbf_column_traits.hpp"

template <size_t HBFSize,
		  typename Column = basic_vhbf_column<HBFSize> >
class vhbf_chunk {
  public:
	typedef Column column_type;
	static const size_t column_height = vhbf_column_traits<Column>::height;
	static const size_t column_count = HBFSize;

	void reset() {
		std::memset(&_columns, 0, sizeof(_columns));
	}

	bool test(size_t row, size_t col) const {
		return _columns[col].test(row);
	}

	void set(size_t row, size_t col) {
		_columns[col].set(row);
	}

	void set_from_hbf(size_t pos, const std::bitset<HBFSize> &hbf) {
		for (size_t i(0); i < column_count; ++i) {
			if (hbf.test(i)) {
				_columns[i].set(pos);
			}
		}
	}

	Column & operator[] (size_t i) {
		return _columns[i];
	}

	const Column & operator[] (size_t i) const {
		return _columns[i];
	}

  private:
	boost::array<Column, column_count> _columns;
};

#endif
