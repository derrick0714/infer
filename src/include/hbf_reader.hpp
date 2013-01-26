#ifndef INFER_INCLUDE_HBF_READER_HPP_
#define INFER_INCLUDE_HBF_READER_HPP_

#include <boost/shared_ptr.hpp>

#include "ErrorStatus.hpp"

template <typename M> 
class hbf_reader {
  public:
	typedef typename M::value_type value_type;

	hbf_reader(boost::shared_ptr<M> read_manager)
		:_read_manager(read_manager)
	{
	}

	ErrorStatus read(value_type &hbf) {
		return _read_manager->read(hbf);
	}

  private:
	boost::shared_ptr<M> _read_manager;
};

#endif
