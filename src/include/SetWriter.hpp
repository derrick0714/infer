#ifndef INFER_INCLUDE_SETWRITER_HPP_
#define INFER_INCLUDE_SETWRITER_HPP_

#include <set>

#include "DataTypeTraits.hpp"

template <typename T>
class SetWriter {
  public:
	typedef writer_type_tag category;
	typedef T value_type;
	
	/// \brief Constructor
	/// \param q the std::set <T> to write to
	explicit SetWriter(std::set <value_type> &s)
		:_s(s)
	{
	}

	/// \brief Write an object
	/// \param obj the object to write
	bool write(const value_type &obj) {
		_s.insert(obj);
		return true;
	}

  private:
	std::set <value_type> &_s;
};

#endif
