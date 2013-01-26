#ifndef OSTREAMWRITER_HPP
#define OSTREAMWRITER_HPP

#include <iosfwd>

#include "DataTypeTraits.hpp"
#include "OstreamHelpers.h"

namespace vn {
namespace arl {
namespace shared {

template <typename T>
class OstreamWriter {
  public:
	typedef writer_type_tag category;
	typedef T value_type;
	
	/// \brief Constructor
	/// \param q the std::ostream <T> to write to
	explicit OstreamWriter(std::ostream &s)
		:_s(s)
	{
	}

	/// \brief Write an object
	/// \param obj the object to write
	bool write(const value_type &obj) {
		_s << obj << std::endl;
		return true;
	}

	operator bool() const {
		return true;
	}

	static std::string error() {
		return std::string();
	}

  private:
	std::ostream &_s;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
