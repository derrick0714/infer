#ifndef INFER_INCLUDE_ZHBF_READ_MANAGER_HPP_
#define INFER_INCLUDE_ZHBF_READ_MANAGER_HPP_

#include "hbf.hpp"
#include "ZlibCompressedHBF.h"
#include "ErrorStatus.hpp"

template <typename ReaderType>
class zhbf_read_manager {
  public:
	typedef hbf value_type;
	static const size_t hbf_size = hbf::hbf_size;

	zhbf_read_manager(boost::shared_ptr<ReaderType> reader)
		:_reader(reader),
		 _zhbf()
	{
	}

	ErrorStatus read(value_type &h) {
		ErrorStatus error_status(_reader->read(_zhbf));
		if (error_status != E_SUCCESS) {
			return error_status;
		}
		
		new(&h) value_type();
		int ret(h.init(_zhbf));
		switch (ret) {
		  case Z_OK:
			break;
		  case Z_MEM_ERROR:
			return E_Z_MEM;
			break;
		  case Z_BUF_ERROR:
			return E_Z_BUF;
			break;
		  case Z_DATA_ERROR:
			return E_Z_DATA;
			break;
		  default:
			return E_UNKNOWN;
			break;
		}

		return E_SUCCESS;
	}

  private:
	boost::shared_ptr<ReaderType> _reader;
	ZlibCompressedHBF _zhbf;
};

#endif
