#include "timeStamp.h"

TimeStamp::TimeStamp() 
    :_seconds(0),
	 _microseconds(0)
{
}

TimeStamp::TimeStamp(const TimeStamp &right)
    :_seconds(right._seconds),
	 _microseconds(right._microseconds)
{
}

TimeStamp::TimeStamp(const timeval &_timeval)
	:_seconds(_timeval.tv_sec),
	 _microseconds(_timeval.tv_usec)
{
}

TimeStamp::TimeStamp(const boost::posix_time::ptime &rhs)
	:_seconds(0),
	 _microseconds(0)
{
	*this = rhs;
}

TimeStamp::TimeStamp(uint32_t seconds, uint32_t microseconds)
    :_seconds(seconds),
	 _microseconds(microseconds)
{
}

TimeStamp &TimeStamp::operator=(const TimeStamp &right) {
  _seconds = right.seconds();
  _microseconds = right.microseconds();
  return *this;
}

TimeStamp &TimeStamp::operator=(const timeval &right) {
  _seconds = right.tv_sec;
  _microseconds = right.tv_usec;
  return *this;
}

TimeStamp &TimeStamp::operator=(const boost::posix_time::ptime &rhs) {
	static const boost::posix_time::ptime epoch
											(boost::gregorian::date(1970,1,1));

	boost::posix_time::time_duration sinceEpoch;
	
	sinceEpoch = rhs - epoch;
	_seconds = sinceEpoch.total_seconds();
	_microseconds = sinceEpoch.fractional_seconds();

	return *this;
}

bool TimeStamp::operator==(const TimeStamp &right) const {
  return (_seconds == right.seconds() && _microseconds == right.microseconds());
}

bool TimeStamp::operator!=(const TimeStamp &right) const {
  return !(*this == right);
}

bool TimeStamp::operator<(const TimeStamp &right) const {
  if (_seconds != right.seconds()) {
    return (_seconds < right.seconds());
  }
  return (_microseconds < right.microseconds());
}

bool TimeStamp::operator>(const TimeStamp &right) const {
  if (_seconds != right.seconds()) {
    return (_seconds > right.seconds());
  }
  return (_microseconds > right.microseconds());
}

bool TimeStamp::operator<=(const TimeStamp &right) const {
  return !(*this > right);
}

bool TimeStamp::operator>=(const TimeStamp &right) const {
  return !(*this < right);
}

TimeStamp &TimeStamp::operator-=(const TimeStamp &right) {
  if (*this <= right) {
    _seconds = 0;
    _microseconds = 0;
    return *this;
  }
  if (_microseconds < right.microseconds()) {
    _microseconds += 1000000;
    _seconds -= 1;
  }
  _microseconds -= right.microseconds();
  _seconds -= right.seconds();
  return *this;
}

const TimeStamp TimeStamp::operator-(const TimeStamp &right) const {
  return TimeStamp(*this) -= right;
}

TimeStamp &TimeStamp::operator+=(const TimeStamp &right) {
  _microseconds += right.microseconds();
  _seconds += right.seconds();
  if (_microseconds >= 1000000) {
    ++_seconds;
    _microseconds -= 1000000;
  }
  return *this;
}

const TimeStamp TimeStamp::operator+(const TimeStamp &right) const {
  return TimeStamp(*this) += right;
}

void TimeStamp::set(uint32_t seconds, uint32_t microseconds) {
  _seconds = seconds;
  _microseconds = microseconds;
}

uint32_t TimeStamp::seconds() const {
  return _seconds;
}

uint32_t TimeStamp::microseconds() const {
  return _microseconds;
}

TimeStamp::operator boost::posix_time::ptime() const {
	static const boost::gregorian::date epoch(1970,1,1);

	return boost::posix_time::ptime(epoch,
									boost::posix_time::time_duration
											(0, 0, _seconds, _microseconds));
}

bool operator< (time_t lhs, const TimeStamp &rhs) {
	if (static_cast<uint32_t>(lhs) != rhs.seconds()) {
		return (static_cast<uint32_t>(lhs) < rhs.seconds());
	}
	return (0 < rhs.microseconds());
}

bool operator> (time_t lhs, const TimeStamp &rhs) {
	if (static_cast<uint32_t>(lhs) != rhs.seconds()) {
		return (static_cast<uint32_t>(lhs) > rhs.seconds());
	}
	return false;
}

bool operator== (time_t lhs, const TimeStamp &rhs) {
	if (static_cast<uint32_t>(lhs) != rhs.seconds()) {
		return false;
	}
	return (0 == rhs.microseconds());
}

bool operator>= (time_t lhs, const TimeStamp &rhs) {
	return lhs > rhs || lhs == rhs;
}

bool operator<= (time_t lhs, const TimeStamp &rhs) {
	return lhs < rhs || lhs == rhs;
}
