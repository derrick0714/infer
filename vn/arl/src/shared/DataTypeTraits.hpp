#ifndef DATATYPETRAITS_HPP
#define DATATYPETRAITS_HPP

#include <bitset>

namespace vn {
namespace arl {
namespace shared {

struct plain_old_data_tag {
	virtual ~plain_old_data_tag() {};
};

struct blob_data_tag : public plain_old_data_tag {};

struct serializable_data_tag {};


struct writer_type_tag {
	virtual ~writer_type_tag() {}
};

struct file_writer_type_tag : public writer_type_tag {
	virtual ~file_writer_type_tag() {}
};

struct reader_type_tag {
	virtual ~reader_type_tag() {}
};

struct file_reader_type_tag : public reader_type_tag {
	virtual ~file_reader_type_tag() {}
};

template <typename T>
struct data_type_traits {
	typedef typename T::data_type data_type;
};

template <size_t N>
struct data_type_traits <std::bitset <N> > {
	typedef plain_old_data_tag data_type;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
