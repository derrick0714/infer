#ifndef INFER_INCLUDE_VHBF_READ_MANAGER_HPP_
#define INFER_INCLUDE_VHBF_READ_MANAGER_HPP_

#include <bitset>

#include <boost/shared_ptr.hpp>

#include "vhbf_chunk.hpp"
#include "fake_hbf.hpp"

template <size_t HBFSize,
		  typename HeaderReader,
		  typename ColumnReader>
class vhbf_read_manager {
  public:
	typedef fake_hbf<vhbf_read_manager> value_type;
	static const size_t hbf_size = HBFSize;

	vhbf_read_manager(boost::shared_ptr<HeaderReader> header_reader,
					  boost::shared_ptr<ColumnReader> column_reader)
		:_chunk(new vhbf_chunk_t),
		 _header_reader(header_reader),
		 _column_reader(column_reader),
		 _loaded_columns(),
		 _next_header(0)
	{
	}

	bool test_bit(size_t row, size_t col) {
		// FIXME somehow pass errors to client
		// test that row is in the same chunk as the last read value_type
		if (row / vhbf_chunk_t::column_height !=
				(_next_header - 1) / vhbf_chunk_t::column_height)
		{
			// error
			abort();
		}

		size_t local_row(row % vhbf_chunk_t::column_height);
		// test if required column is loaded, read it if not
		if (!_loaded_columns.test(col)) {
			size_t abs_col(((row / vhbf_chunk_t::column_height) * HBFSize) + col);
			ErrorStatus error_status(_column_reader->read((*_chunk)[col],
														  abs_col));
			if (error_status != E_SUCCESS) {
				abort();
			}
			_loaded_columns.set(col);
			// std::cerr << "(row, abs_col) = (" << row << ", " << abs_col << ")" << std::endl;
		}

		return _chunk->test(local_row, col);
	}

	ErrorStatus read(value_type &hbf) {
		ErrorStatus error_status(_header_reader->read(hbf._vhbf_header));
		if (error_status != E_SUCCESS) {
			return error_status;
		}
		if (_next_header % vhbf_chunk_t::column_height == 0) {
			_loaded_columns.reset();
		}
		hbf._row = _next_header;
		++_next_header;
		hbf._v = this;

		return E_SUCCESS;
	}

  private:
	typedef vhbf_chunk<HBFSize, typename ColumnReader::value_type>
		vhbf_chunk_t;

	boost::shared_ptr<vhbf_chunk_t> _chunk;

	boost::shared_ptr<HeaderReader> _header_reader;
	boost::shared_ptr<ColumnReader> _column_reader;

	std::bitset<HBFSize> _loaded_columns;
	size_t _next_header;
};

#endif
