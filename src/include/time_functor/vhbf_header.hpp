#ifndef INFER_INCLUDE_TIME_FUNCTOR_VHBF_HEADER_HPP_
#define INFER_INCLUDE_TIME_FUNCTOR_VHBF_HEADER_HPP_

#include "include/time_functor.hpp"
#include "include/vhbf_header.hpp"

template <>
struct time_functor<vhbf_header> {
	TimeStamp operator() (const vhbf_header &header) const {
		return header.start_time();
	}
}

#endif
