#ifndef INFER_INCLUDE_DATATYPETRAITS_HPP_
#define INFER_INCLUDE_DATATYPETRAITS_HPP_

#include <bitset>

struct plain_old_data_tag {};

struct blob_data_tag {};

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
	static uint8_t TypeID() {
		return T::TypeID;
	}
};

template <size_t N>
struct data_type_traits <std::bitset <N> > {
	typedef plain_old_data_tag data_type;
};

#endif
