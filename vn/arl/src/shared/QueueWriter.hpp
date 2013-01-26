#ifndef QUEUEWRITER_HPP
#define QUEUEWRITER_HPP

#include <queue>

#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename T>
class QueueWriter {
  public:
	typedef writer_type_tag category;
	typedef T value_type;
	
	/// \brief Constructor
	/// \param q the std::queue <T> to write to
	explicit QueueWriter(std::queue <value_type> &q)
		:_q(q)
	{
	}

	/// \brief Write an object
	/// \param obj the object to write
	bool write(const value_type &obj) {
		_q.push(obj);
		return true;
	}

  private:
	std::queue <value_type> &_q;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
