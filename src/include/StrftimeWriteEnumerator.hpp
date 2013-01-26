#ifndef INFER_INCLUDE_STRFTIMEWRITEENUMERATOR_HPP_
#define INFER_INCLUDE_STRFTIMEWRITEENUMERATOR_HPP_

#include <ctime>
#include <cstdio>

#include <boost/filesystem/path.hpp>

#include "time_functor.hpp"

template <typename T, typename F = time_functor<T> >
class StrftimeWriteEnumerator {
  public:
	typedef uint32_t index_type;

	explicit StrftimeWriteEnumerator(const boost::filesystem::path &baseDir,
									 const std::string &format,
									 const F &f = F());

	boost::filesystem::path getFileName(const T &obj) const;

	index_type getIndex(const T &obj) const;

  private:
	boost::filesystem::path _getFileName(time_t t) const;

	const boost::filesystem::path _baseDir;
	const std::string _format;
	uint32_t _interval;
	F _f;
};

template <typename T, typename F>
StrftimeWriteEnumerator<T, F>::
StrftimeWriteEnumerator(const boost::filesystem::path &baseDir,
						const std::string &format,
						const F &f)
	:_baseDir(baseDir),
	 _format(format),
	 _interval(0),
	 _f(f)
{
	// determine interval for getIndex().
	boost::filesystem::path epochPath(_getFileName(0));

	// try seconds
	_interval = 1;
	if (_getFileName(_interval) != epochPath) {
		return;
	}
	
	// minutes
	_interval = 60;
	if (_getFileName(_interval) != epochPath) {
		return;
	}

	// hours
	_interval = 3600;
	if (_getFileName(_interval) != epochPath) {
		return;
	}

	// days
	_interval = 86400;
	if (_getFileName(_interval) != epochPath) {
		return;
	}

	// longer intervals not supported. exception on getIndex
	_interval = 0;
}

template <typename T, typename F>
boost::filesystem::path 
StrftimeWriteEnumerator<T, F>::getFileName(const T &obj) const {
	return _getFileName(static_cast<time_t>(_f(obj).seconds()));
}

template <typename T, typename F>
typename StrftimeWriteEnumerator<T, F>::index_type
StrftimeWriteEnumerator<T, F>::getIndex(const T &obj) const {
	return (_f(obj).seconds() / _interval) * _interval;
}

template <typename T, typename F>
boost::filesystem::path StrftimeWriteEnumerator<T, F>::_getFileName(time_t t) const {
	static tm _tm;
	static char buf[FILENAME_MAX];
	static size_t _len;

	gmtime_r(&t, &_tm);
	_len = strftime(buf, sizeof(buf), _format.c_str(), &_tm);

	if (_len != 0) {
		return boost::filesystem::path(_baseDir) /= std::string(buf, _len);
	}

	return boost::filesystem::path();
}

#endif
