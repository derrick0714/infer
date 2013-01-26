#ifndef STRFTIMEWRITEENUMERATOR_HPP
#define STRFTIMEWRITEENUMERATOR_HPP

#include <ctime>
#include <cstdio>
#include <boost/filesystem/path.hpp>

namespace vn {
namespace arl {
namespace shared {

namespace fs = boost::filesystem;

template <typename T>
class StrftimeWriteEnumerator {
  public:
	explicit StrftimeWriteEnumerator(const fs::path &baseDir,
									 const std::string &format);

	boost::filesystem::path getFileName(const T &obj);

  private:
	const boost::filesystem::path _baseDir;
	const std::string _format;
	char buf[FILENAME_MAX];
	size_t len;
};

template <typename T>
StrftimeWriteEnumerator<T>::StrftimeWriteEnumerator(const fs::path &baseDir,
													const std::string &format)
	:_baseDir(baseDir),
	 _format(format),
	 len(0)
{
}

template <typename T>
boost::filesystem::path StrftimeWriteEnumerator<T>::getFileName(const T &obj) {
	tm _tm;
	time_t t(static_cast<time_t>(obj.time().seconds()));

	gmtime_r(&t, &_tm);
	len = strftime(buf, sizeof(buf), _format.c_str(), &_tm);

	if (len != 0) {
		return fs::path(_baseDir) /= std::string(buf, len);
	}

	return fs::path();
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
