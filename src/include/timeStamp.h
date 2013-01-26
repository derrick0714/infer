#ifndef INFER_INCLUDE_TIMESTAMP_H_
#define INFER_INCLUDE_TIMESTAMP_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <sys/time.h>

class TimeStamp {
  public:
	/// \brief Default constructor
    TimeStamp();

	/// \brief Copy constructor
	/// \param right the TimeStamp to copy
    TimeStamp(const TimeStamp &right);

	/// \brief Construct from timeval
	/// \param _timeval the struct timeval to construct from
    TimeStamp(const timeval &_timeval);

	/// \brief Construct from boost::posix_time::ptime
	/// \param rhs the boost::posix_time::ptime to construct from
    TimeStamp(const boost::posix_time::ptime &rhs);

	/// \brief Construct from seconds and microseconds
	/// \brief seconds number of seconds to initialize with
	/// \brief microseconds number of microseconds to initialize with
    TimeStamp(uint32_t seconds, uint32_t microseconds);

	/// \brief Copy assignment operator
	/// \param right the TimeStamp to assign from
	/// \returns *this
    TimeStamp &operator=(const TimeStamp &right);

	/// \brief Assign from a timeval
	/// \param right the struct timeval to assign from
	/// \returns *this
    TimeStamp &operator=(const timeval &right);

	/// \bief Assign from a boost::posix_time::ptime
	/// \param rhs the boost::posix_time::ptime to assign from
    TimeStamp &operator=(const boost::posix_time::ptime &rhs);

	/// \brief Equality comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this and rhs represent the same time
    bool operator==(const TimeStamp &rhs) const;

	/// \brief Negated equality comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this and rhs represent different times
    bool operator!=(const TimeStamp &rhs) const;

	/// \brief less-than comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this represents a shorter amount of time than rhs
    bool operator<(const TimeStamp &rhs) const;

	/// \brief greater-than comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this represents a longer amount of time than rhs
    bool operator>(const TimeStamp &rhs) const;

	/// \brief less-than-or-equal-to comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this represents either a shorter or the same amount
	/// of time as rhs
    bool operator <=(const TimeStamp &rhs) const;

	/// \brief greater-than comparison operator
	/// \param rhs the TimeStamp to compare to
	/// \returns true if *this represents either a longer or the same amount of
	/// time as rhs
    bool operator >=(const TimeStamp &rhs) const;

	/// \brief subtraction operator
	/// \prarm rhs the TimeStamp to subtract
	/// \returns A TimeStamp representing the difference in times represented
	/// by *this and rhs
    const TimeStamp operator-(const TimeStamp &rhs) const;

	/// \brief subtraction assignment operator
	/// \prarm rhs the TimeStamp to subtract from *this
	/// \returns *this
    TimeStamp &operator-=(const TimeStamp &rhs);

	/// \brief addition operator
	/// \prarm rhs the TimeStamp to add
	/// \returns A TimeStamp representing the sum of the times represented by
	/// *this and rhs
    const TimeStamp operator+(const TimeStamp &rhs) const;

	/// \brief addition assignment operator
	/// \prarm rhs the TimeStamp to add to *this
	/// \returns *this
    TimeStamp &operator+=(const TimeStamp &rhs);

	/// \brief Assign a new value
	/// \param seconds the seconds to assign
	/// \param microseconds the microseconds to assign
    void set(uint32_t seconds, uint32_t microseconds);

	/// \brief Get the number of seconds
	/// \returns the seconds portion of *this
    uint32_t seconds() const;

	/// \brief Get the number of microseconds
	/// \returns the microseconds portion of *this
    uint32_t microseconds() const;

	/// \brief boost::posix_time::ptime cast operator
	/// \returns a boost::posix_time::ptime representing the same time as *this
	operator boost::posix_time::ptime() const;

  private:
  	/// the number of seconds
    uint32_t _seconds;

	/// the number of microseconds
    uint32_t _microseconds;
} __attribute__ ((packed));

bool operator< (time_t lhs, const TimeStamp &rhs);
bool operator> (time_t lhs, const TimeStamp &rhs);
bool operator== (time_t lhs, const TimeStamp &rhs);
bool operator<= (time_t lhs, const TimeStamp &rhs);
bool operator>= (time_t lhs, const TimeStamp &rhs);

#endif
