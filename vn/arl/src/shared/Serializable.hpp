#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

#include <string>

#include "TimeStamp.h"

namespace vn {
namespace arl {
namespace shared {

/// \brief Abstract base class for serializable objects
///
/// Any data type that will be read using a FileReader or written using
/// a FileWriter must inherit from this class.
template <typename SerializableType>
class Serializable {
  public:
	/// \brief A typedef for the size_type of the serialized data
	typedef std::string::size_type size_type;

	/// \brief Virtual destructor
	virtual ~Serializable() {};

	/// \brief Get the start time of the data
	/// \returns the start time of the data
  	virtual TimeStamp startTime() const = 0;
	
	/// \brief Get the end time of the data
	/// \returns the end time of the data
  	virtual TimeStamp endTime() const = 0;

	/// \brief Get the size of the the serialized data
	/// \returns the size of the serialized data
	virtual size_type size() const = 0;

	/// \brief Serialize data
	/// \param ostr the string in which to store the serialized data
	/// \returns true if the data was successfully serialized into ostr
  	virtual bool serialize(std::string &ostr) const = 0;

	/// \brief Unserialize data
	/// \param istr the string from which unserialize data
	/// \returns true if the data was successfully unserialized from ostr
	virtual bool unserialize(const std::string &istr) = 0;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
