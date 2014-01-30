#ifndef STRFTIMEREADENUMERATOR_HPP
#define STRFTIMEREADENUMERATOR_HPP

#include <ctime>
#include <cstdio>
#include <boost/filesystem.hpp>

#include "timeStamp.h"

namespace fs = boost::filesystem;

class StrftimeReadEnumerator {
  public:
	typedef std::vector <fs::path> Container;
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;
	typedef Container::size_type size_type;
	typedef Container::value_type value_type;

	explicit StrftimeReadEnumerator();
	explicit StrftimeReadEnumerator(const fs::path &baseDir,
									const std::string &format,
									const TimeStamp &startTime,
									const TimeStamp &endTime);

	void init(const fs::path &baseDir,
			  const std::string &format,
			  const TimeStamp &startTime,
			  const TimeStamp &endTime);

	iterator begin() {
		return _container.begin();
	}

	iterator end() {
		return _container.end();
	}

	const_iterator begin() const {
		return _container.begin();
	}

	const_iterator end() const {
		return _container.end();
	}

	size_type size() {
		return _container.size();
	}

	operator bool() const {
		return !_error;
	}

	std::string error() const {
		return _errorMsg;
	}

  private:
	Container _container;
	
	bool _error;

	std::string _errorMsg;
};

inline StrftimeReadEnumerator::StrftimeReadEnumerator()
	:_container(),
	 _error(false),
	 _errorMsg()
{
}

inline
StrftimeReadEnumerator::StrftimeReadEnumerator(const fs::path &baseDir,
											   const std::string &format,
											   const TimeStamp &startTime,
											   const TimeStamp &endTime)
	:_container(),
	 _error(false),
	 _errorMsg()
{
	init(baseDir, format, startTime, endTime);
}

void StrftimeReadEnumerator::init(const fs::path &baseDir,
								  const std::string &format,
								  const TimeStamp &startTime,
								  const TimeStamp &endTime)
{
	char buf[FILENAME_MAX];
	size_t len;
	fs::path prevFile, curFile;
	tm _tm;
	time_t start(static_cast<time_t>(startTime.seconds()));
	time_t t(start);

	_container.clear();

	// first, determine the time difference between files
	gmtime_r(&t, &_tm);
	len = strftime(buf, sizeof(buf), format.c_str(), &_tm);
	if (len == 0) {
		// error
		_error = true;
		_errorMsg.assign("strftime() error");
		return;
	}
	curFile = prevFile = std::string(buf, len);
	//printf("#######:current file :%s\r\n##########",curFile.c_str());
	if (fs::exists(fs::path(baseDir) /= curFile) && 
		fs::is_regular_file(fs::path(baseDir) /= curFile))
	{
		_container.push_back(fs::path(baseDir) /= curFile);
	}

	// this can be more efficient...TODO later on, don't check every second
	while (prevFile == curFile) {
		++t;
		if (t - start >= endTime - startTime) {
			return;
		}

		++_tm.tm_sec;
		gmtime_r(&t, &_tm);
		len = strftime(buf, sizeof(buf), format.c_str(), &_tm);
		if (len == 0) {
			// error
			_error = true;
			_errorMsg.assign("strftime() error");
			return;
		}
		curFile = std::string(buf, len);
	}
	if (fs::exists(fs::path(baseDir) /= curFile) && 
		fs::is_regular_file(fs::path(baseDir) /= curFile))
	{
		_container.push_back(fs::path(baseDir) /= curFile);
	}
	time_t secondStart(t);
	prevFile = curFile;
	while (prevFile == curFile) {
		++t;
		if (t - start >= endTime - startTime) {
			return;
		}

		++_tm.tm_sec;
		gmtime_r(&t, &_tm);
		len = strftime(buf, sizeof(buf), format.c_str(), &_tm);
		if (len == 0) {
			// error
			_error = true;
			_errorMsg.assign("strftime() error");
			return;
		}
		curFile = std::string(buf, len);
	}
	time_t diff(t - secondStart);

	// how many files?
	uint32_t numFiles(endTime.seconds() - startTime.seconds());
	if (endTime.microseconds() > startTime.microseconds()) {
		++numFiles;
	}
	numFiles = 
		ceil(static_cast <double>(numFiles) / static_cast <uint32_t>(diff));

	if (numFiles <= 2) {
		return;
	}

	if (fs::exists(fs::path(baseDir) /= curFile) && 
		fs::is_regular_file(fs::path(baseDir) /= curFile))
	{
		_container.push_back(fs::path(baseDir) /= curFile);
	}

	t += diff;
	for (uint32_t i = 3; i < numFiles; ++i, t += diff) {
		gmtime_r(&t, &_tm);
		len = strftime(buf, sizeof(buf), format.c_str(), &_tm);
		if (len == 0) {
			// error
			_error = true;
			_errorMsg.assign("strftime() error");
			return;
		}

		curFile = baseDir;
		curFile /= std::string(buf, len);

		if (fs::exists(curFile) && fs::is_regular_file(curFile)) {
			_container.push_back(curFile);
		}
	}
}

#endif
