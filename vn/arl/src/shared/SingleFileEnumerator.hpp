#ifndef SINGLEFILEENUMERATOR_HPP
#define SINGLEFILEENUMERATOR_HPP

#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "FileEnumerator.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A single file FileEnumerator
///
/// This FileEnumerator provides means reading just one file by classes
/// that expect enumeration. The default container it will use is
/// std::vector <boost::filesystem::path>
///
///	\sa FileEnumerator
///
template <typename Container = std::vector <boost::filesystem::path> >
class _SingleFileEnumerator : public FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type> {
  public:
	/// \brief A typedef for the type of iterator to iterate through files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::iterator iterator;

	/// \brief A typedef for the type of const_iterator to iterate through files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::const_iterator const_iterator;

	/// \brief A typedef for the size_type of the container used to hold the
	/// list of files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::size_type size_type;

	explicit _SingleFileEnumerator(boost::filesystem::path file);

	/// \brief Get the number of files
	/// \returns the number of files
	size_type size() const;

	/// \brief Get an iterator at the beginning of the files
	/// \returns an iterator at the beginning of the files
	iterator begin();

	/// \brief Get an iterator past the last file
	/// \returns an iterator past the lat file
	iterator end();

	/// \brief Get a const_iterator at the beginning of the files
	/// \returns a const_iterator at the beginning of the files
	const_iterator begin() const;

	/// \brief Get a const_iterator past the last file
	/// \returns a const_iterator past the lat file
	const_iterator end() const;
	
  private:
	/// The files
	Container files;
};

/// \brief A typedef for conveniently creating a _FilteredFileEnumerator with
/// the default container type
typedef _SingleFileEnumerator <> SingleFileEnumerator;

template <typename Container>
_SingleFileEnumerator <Container>::_SingleFileEnumerator(boost::filesystem::path file) {
    files.push_back(file);
    
    this->isInitialized = true;
}

template <typename Container>
typename _SingleFileEnumerator <Container>::size_type _SingleFileEnumerator <Container>::size() const {
	return files.size();
}

template <typename Container>
typename _SingleFileEnumerator <Container>::iterator _SingleFileEnumerator <Container>::begin() {
	return files.begin();
}

template <typename Container>
typename _SingleFileEnumerator <Container>::iterator _SingleFileEnumerator <Container>::end() {
	return files.end();
}

template <typename Container>
typename _SingleFileEnumerator <Container>::const_iterator _SingleFileEnumerator <Container>::begin() const {
	return files.begin();
}

template <typename Container>
typename _SingleFileEnumerator <Container>::const_iterator _SingleFileEnumerator <Container>::end() const {
	return files.end();
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif

