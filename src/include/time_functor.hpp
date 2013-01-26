#ifndef INFER_INCLUDE_TIME_FUNCTOR_HPP_
#define INFER_INCLUDE_TIME_FUNCTOR_HPP_

#include "timeStamp.h"

template <typename T>
struct time_functor {
	TimeStamp operator() (const T &obj) const {
		return obj.time();
	}
};

#endif
