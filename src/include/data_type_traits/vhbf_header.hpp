#ifndef INFER_INCLUDE_DATA_TYPE_TRAITS_VHBF_HEADER_HPP_
#define INFER_INCLUDE_DATA_TYPE_TRAITS_VHBF_HEADER_HPP_

#include "../DataTypeTraits.hpp"
#include "../vhbf_header.hpp"

template <>
struct data_type_traits<vhbf_header> {
	typedef plain_old_data_tag data_type;
	static uint8_t TypeID() {
		return 0x02;
	}
};

#endif
