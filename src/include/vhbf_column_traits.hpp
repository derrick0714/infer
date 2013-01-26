#ifndef INFER_INCLUDE_VHBF_COLUMN_TRAITS_HPP_
#define INFER_INCLUDE_VHBF_COLUMN_TRAITS_HPP_

template <typename T>
struct vhbf_column_traits {
	static const size_t height = T::height;
};

#endif
