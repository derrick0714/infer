#ifndef FILTEREDFILEENUMERATOR_HPP
#define FILTEREDFILEENUMERATOR_HPP

#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "FileEnumerator.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A regular expression based FileEnumerator
///
/// This FileEnumerator provides means means to iterate all files with specified pattern
/// in a directory. The default container it will use is
/// std::vector <boost::filesystem::path>
///
///	\sa FileEnumerator
///
template <typename Container = std::vector <boost::filesystem::path> >
class _FilteredFileEnumerator : public FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type> {
  public:
	/// \brief A typedef for the type of iterator to iterate through files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::iterator iterator;

	/// \brief A typedef for the type of const_iterator to iterate through files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::const_iterator const_iterator;

	/// \brief A typedef for the size_type of the container used to hold the
	/// list of files
	typedef typename FileEnumerator <typename Container::iterator, typename Container::const_iterator, typename Container::size_type>::size_type size_type;

	/// \todo Also provide the value_type. It would be easy to privide support
	/// for wstring file names later when we have the need. In addition, any
	/// assumption of type (like how PcapFileReader assumes this enumerator
	/// returns a path() are made explicit to the compiler/reader cleanly.

	/// \brief Initialize the _FilteredFileEnumerator
	/// \param dir The directory in which to recursively enumerate files
	/// \returns true if the _FilteredFileEnumerator was successfully
	/// initialized
	bool init(boost::filesystem::path dir, const char* rx);

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
typedef _FilteredFileEnumerator <> FilteredFileEnumerator;

template <typename Container>
bool _FilteredFileEnumerator <Container>::init(boost::filesystem::path dir, const char* rx) {
	if (!is_directory(dir)) {
		return false;
	}

	boost::regex re(rx, boost::regex::perl);
	boost::cmatch what; 

	/// \remark Why bother with storing in the vector? Why not directly use
	/// this call in begin() and end()? We have to iterate through the loop
	/// below for knowing the size() but we will save on space by not storing the
	/// path names in a vector.
	/// 
	/// \remark An added benefit of not storing the paths in a vector is that
	/// the changes to the directory structures between calls are reflected (if
	/// reflected by boost::filesystem)
	///
	/// \remark Yes, I just copied _RecursiveFileEnumerator. This class should
	/// inherit instead and override init, this would require 'files' field
	/// to be protected.
	for (boost::filesystem::recursive_directory_iterator it(dir);
		 it != boost::filesystem::recursive_directory_iterator();
		 ++it)
	{
		if (boost::filesystem::is_regular_file(it -> status())) {
			std::string filename = it -> path() . file_string();
			const char* cstr = filename.c_str();

			if(boost::regex_match(cstr, re)) {
				files.push_back(filename);
			}
		}
	}

	this -> isInitialized = true;
	return true;
}

template <typename Container>
typename _FilteredFileEnumerator <Container>::size_type _FilteredFileEnumerator <Container>::size() const {
	return files.size();
}

template <typename Container>
typename _FilteredFileEnumerator <Container>::iterator _FilteredFileEnumerator <Container>::begin() {
	return files.begin();
}

template <typename Container>
typename _FilteredFileEnumerator <Container>::iterator _FilteredFileEnumerator <Container>::end() {
	return files.end();
}

template <typename Container>
typename _FilteredFileEnumerator <Container>::const_iterator _FilteredFileEnumerator <Container>::begin() const {
	return files.begin();
}

template <typename Container>
typename _FilteredFileEnumerator <Container>::const_iterator _FilteredFileEnumerator <Container>::end() const {
	return files.end();
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
