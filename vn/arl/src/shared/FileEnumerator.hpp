#ifndef FILEENUMERATOR_HPP
#define FILEENUMERATOR_HPP

namespace vn {
namespace arl {
namespace shared {

/// \class FileEnumerator FileEnumerator.hpp
/// \brief Abstract base class for a FileEnumerator
///
/// A FileEnumerator is a directory structure aware class. It makes the
/// uderlying file system organization and temporal characteristics of 
/// files transparent to any class that uses these enumerators. Any 
/// FileEnumerator must inherit from this class. A FileEnumerator is used
/// by a FileReader to iterate through the files that are to be read.
///
/// \sa FileReader
///
template <typename Iterator, typename ConstIterator, typename SizeType>
class FileEnumerator {
  public:
	/// \brief A typedef for the type of iterator to iterate through files
	typedef Iterator iterator;

	/// \brief A typedef for the type of const_iterator to iterate throuh files
	typedef ConstIterator const_iterator;

	/// \brief A typedef for the size_type of the container used to hold the
	/// list of files
	typedef SizeType size_type;

	/// \brief Constructor
	///
	/// Initializes isInitialized to false.
	FileEnumerator()
		:isInitialized(false)
	{
	}

	/// \brief Virtual destructor
	virtual ~FileEnumerator() {
	}

	/// \brief Get the number of files
	/// \returns the number of files
	virtual size_type size() const = 0;

	/// \brief Get an iterator at the beginning of the files
	/// \returns an iterator at the beginning of the files
	virtual iterator begin() = 0;

	/// \brief Get an iterator past the last file
	/// \returns an iterator past the lat file
	virtual iterator end() = 0;

	/// \brief Get a const_iterator at the beginning of the files
	/// \returns a const_iterator at the beginning of the files
	virtual const_iterator begin() const = 0;

	/// \brief Get a const_iterator past the last file
	/// \returns a const_iterator past the lat file
	virtual const_iterator end() const = 0;

	/// \brief Not operator
	/// \returns true if the FileEnumerator has not been successfully
	/// initialized
	bool operator!() const {
		return !isInitialized;
	}

	/// \brief Cast operator for bool
	/// \returns true if the FileEnumerator has been successfully initialized
	operator bool() const {
		return isInitialized;
	}

  protected:
	/// A boolean value representing whether or not this FileEnumerator is
	/// ready to be used
	bool isInitialized;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
