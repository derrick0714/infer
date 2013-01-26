#ifndef RECURSIVEREADENUMERATOR_HPP
#define RECURSIVEREADENUMERATOR_HPP

#include <ctime>
#include <cstdio>
#include <boost/filesystem.hpp>

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

class RecursiveReadEnumerator {
  public:
	typedef fs::recursive_directory_iterator iterator;
	typedef iterator const_iterator;
	typedef iterator::path_type value_type;

	explicit RecursiveReadEnumerator(const fs::path &baseDir);

	iterator begin() {
		return iterator(_baseDir);
	}

	iterator end() {
		return iterator();
	}

	const_iterator begin() const {
		return const_iterator(_baseDir);
	}

	const_iterator end() const {
		return const_iterator();
	}

	operator bool() const {
		return !_error;
	}

	std::string error() const {
		return _errorMsg;
	}

  private:
	fs::path _baseDir;
	
	bool _error;

	std::string _errorMsg;
};

RecursiveReadEnumerator::RecursiveReadEnumerator(const fs::path &baseDir)
	:_baseDir(baseDir),
	 _error(false),
	 _errorMsg()
{
	if (!is_directory(baseDir)) {
		_error = true;
		_errorMsg.assign("'");
		_errorMsg.append(baseDir.file_string() + "' is not a directory");
	}
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
