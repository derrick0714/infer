#ifndef INCLUDE_INTERVAL_HPP
#define INCLUDE_INTERVAL_HPP

/// \class interval interval.hpp
/// \brief Represents a range of data.
///
/// This class is used to represent a range of blocks on a digital media
/// source. In addition, the class also implements a set of operators that
/// can be used to easily compare block ranges. The block ranges created
/// and represented by interval are forward-looking. For example, [0, 10]
/// and [4, 4] are correct forward-looking ranges whereas [5, 2] is not.
/// See BlockLayout for more information.
///
/// \remark We could have used Boost Intervals but that seems to be little
/// too much for our current needs.
///
template <typename T>
class interval {
  public:
	/// \typedef T value_type
	/// A type definition for the bounds of block range.
	typedef T value_type;

	/// Constructor
	/// Default Constructor
	interval()
		:_begin(0),
		 _end(0)
	{}

	/// \param (br) An instance of interval.
	/// \brief Copy constructor for interval class.
	interval(const interval &br)
		:_begin(br._begin),
		 _end(br._end)
	{}


	/// \name Accessors
	/// Getter and setter methods for interval.
	//@{
	///
	/// \param (b) Beginning of a block range.
	/// \param (e) End of the block range.
	/// \return Returns true when the bounds are assigned to the block range; False otherwise.
	/// \brief Sets a new range.
	bool set(const value_type &b, const value_type &e) {
		if (e < b) {
			return false;
		}

		_begin = b;
		_end = e;
		return true;
	}

	/// \return Returns the Block Id of the beginning of the block range.
	/// \brief Retruns the Block Id of the beginning of the block range.
	value_type begin() const {
		return _begin;
	}

	///	\param (b) A beginning for block range.
	/// \return Returns true once a valid beginning is set; False when (b) is invalid.
	/// \brief Sets a new beginning for a block range given a valid beginning.
	bool begin(const value_type &b) {
		if (_end < b) {
			return false;
		}

		_begin = b;
		return true;
	}

	/// \return Returns the Block Id of the end of the block range.
	/// \brief Retruns the Block Id of the end of the block range.
	value_type end() const {
		return _end;
	}

	///	\param (e) An end for block range.
	/// \return Returns true once a valid end is set; False when (e) is invalid.
	/// \brief Sets a new end for a block range given a valid end.
	bool end(const value_type &e) {
		if (e < _begin) {
			return false;
		}

		_end = e;
		return true;
	}
	//@}

	bool contains(const value_type &v) const {
		return _begin <= v && v <= _end;
	}

	/// \name Overloaded Operators
	/// Arithmetic operators overloaded for a pair of intervals
	//@{
	/// \param (b) Another interval to assign.
	/// \return Returns a reference to a newly assigned interval
	/// instance.
	/// \brief Implements the assignment operator for interval.
	/* // shallow copy is ok here...use default assignment op.
	interval& operator= (const interval& b) {
		_begin = b._begin;
		_end = b._end;
		return *this;
	}
	*/

	/// \param (b) Another interval to compare with.
	/// \return Returns true when the entire block range (b) is behind
	/// this block range; False otherwise.
	/// \brief Implements the less-than operator on a pair of intervals.
	///
	/// Given two block ranges \f$[I_i, I_j], [I_k, I_l]\f$ this
	/// operator returns true if and only if \f$I_j < I_k\f$; false
	/// otherwise. In other words, it returns true if and only if the
	/// entire interval \f$[I_i, I_j]\f$ is in front of interval
	/// \f$[I_k, I_l]\f$.
	bool operator< (const interval& b) const {
		return _end < b._begin;
	}

	/// \param (b) Another interval to compare with.
	/// \return Returns true when the entire block range (b) is in
	/// front of this block range; False otherwise.
	/// \brief Implements the greater-than operator on a pair of intervals.
	///
	/// Given two block ranges \f$[I_i, I_j], [I_k, I_l]\f$ this
	/// operator returns true if and only if \f$I_i > I_l\f$; false
	/// otherwise. In other words, it returns true if and only if the
	/// entire interval \f$[I_i, I_j]\f$ is behind the interval
	/// \f$[I_k, I_l]\f$.
	bool operator> (const interval& b) const {
		return _begin > b._end;
	}

	/// \param (b) Another interval to compare with.
	/// \return Returns true when the block range (b)is exactly on top
	/// of this block; False otherwise.
	/// \brief Implements the equals operator on a pair of intervals.
	///
	/// Given two block ranges \f$[I_i, I_j], [I_k, I_l]\f$ this
	/// operator returns true if and only if
	/// \f$\left(\left(I_i == I_k\right) {\bf and} \left(I_j == I_l\right)\right)\f$;
	/// false otherwise.
	bool operator== (const interval& b) const {
		return _begin == b._begin && _end == b._end;
	}

	/// \param (b) Another interval to compare with.
	/// \return Returns true when the block range (b) is not on top of
	/// this block range; False otherwise.
	/// \brief Implements the equals operator on a pair of intervals.
	///
	/// Given two block ranges \f$[I_i, I_j], [I_k, I_l]\f$ this
	/// operator returns true if and only if
	/// \f$\left(\left(I_i != I_k\right) {\bf or } \left(I_j != I_l\right)\right)\f$;
	/// false otherwise.
	bool operator!= (const interval& b) const {
		return ! (*this == b);
	}

  private:
	value_type _begin;	/// Beginning of the block range.
	value_type _end;	/// End of the block range.
};

#endif
