#ifndef INFER_INCLUDE_OSTREAM_WRITER_HPP_
#define INFER_INCLUDE_OSTREAM_WRITER_HPP_

#include <iosfwd>

#include "DataTypeTraits.hpp"
#include "OstreamHelpers.h"
#include "ErrorStatus.hpp"

template <typename T>
class ostream_writer {
  public:
	typedef writer_type_tag category;
	typedef T value_type;
	
	/// \brief Constructor
	/// \param q the std::ostream <T> to write to
	explicit ostream_writer(std::ostream &s)
		:_s(s)
	{
	}

	ErrorStatus write(const value_type &obj) {
		_s << obj << std::endl;
		return E_SUCCESS;
	}

  private:
	std::ostream &_s;
};

#endif
