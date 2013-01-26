#ifndef INFER_INCLUDE_FIXED_TIME_FUNCTOR_HPP_
#define INFER_INCLUDE_FIXED_TIME_FUNCTOR_HPP_

#include "timeStamp.h"

struct fixed_time_functor {
	fixed_time_functor(const TimeStamp &t)
		:_t(t)
	{
	}

	template<typename T>
	TimeStamp operator() (const T&) const {
		return _t;
	}

  private:
	const TimeStamp _t;
};

#endif
