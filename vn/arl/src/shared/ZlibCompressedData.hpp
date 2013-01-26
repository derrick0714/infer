#ifndef ZLIBCOMPRESSEDDATA_HPP
#define ZLIBCOMPRESSEDDATA_HPP

#include <cmath>
#include <zlib.h>

#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \todo handle errors
template <typename T>
class ZlibCompressedData {
  public:
	/// \brief A typedef for the DataTypeTrait
	typedef blob_data_tag data_type;

	/// \brief A typedef for the type that is compressed
	typedef T value_type;

	/// \brief the size of the contained data buffer
	static const size_t DataSize = sizeof(T) +
								   static_cast<size_t>(sizeof(T) * 0.001) + 13;

	/// \brief Constructor
	ZlibCompressedData();

	/// \brief Compress obj
	/// \note Only plain_old_data are supported right now
	int compress(const T &obj);

	/// \brief Uncompress into obj
	/// \note Only plain_old_data are supported right now
	///
	/// Uncompresses the compressed data in this object into obj
	int uncompress(T &obj) const;

	/// \brief Get a pointer to the beginning of the compressed data
	/// \returns a pointer to the beginning of the compressed data
	char * data() {
		return _data;
	}

	/// \brief Get the size of the compressed data
	/// \returns the size of the compressed data
	size_t size() const;

	/// \brief Set the size of the compressed data
	/// \param _size the size to set
	void size(size_t _size);

  private:
	/// \brief Compress an object that's plain_old_data
	/// \param obj the object to compress
	int _compress(const T &obj, plain_old_data_tag);

	/// \brief Uncompress into an object that's plain_old_data
	/// \param obj the object to uncompress into
	int _uncompress(T &obj, plain_old_data_tag) const;

	// FIXME Why the fuck won't this compile? -- now we're possibly 
	// wasting 1 byte
	//char _data[sizeof(T) + static_cast<size_t>(ceil(sizeof(T) * 0.001)) + 12];
	/// The compression buffer
	char _data[sizeof(T) + static_cast<size_t>(sizeof(T) * 0.001) + 13];

	/// The size of the compressed data in the buffer
	size_t _size;
} __attribute__ ((packed));

template <typename T>
ZlibCompressedData<T>::ZlibCompressedData()
	:_size(0)
{
}

template <typename T>
int ZlibCompressedData<T>::compress(const T &obj) {
	return _compress(obj, typename data_type_traits<T>::data_type());
}

template <typename T>
int ZlibCompressedData<T>::uncompress(T &obj) const {
	return _uncompress(obj, typename data_type_traits<T>::data_type());
}

template <typename T>
size_t ZlibCompressedData<T>::size() const {
	return _size;
}

template <typename T>
void ZlibCompressedData<T>::size(size_t size) {
	_size = size;
}

template <typename T>
int ZlibCompressedData<T>::_compress(const T &obj, plain_old_data_tag)
{
	_size = sizeof(_data);
	return ::compress(reinterpret_cast<Bytef*>(_data),
					  reinterpret_cast<uLongf*>(&_size),
					  reinterpret_cast<const Bytef*>(&obj),
					  sizeof(T));
}

template <typename T>
int ZlibCompressedData<T>::_uncompress(T &obj, plain_old_data_tag) const
{
	uLong tmpSize = sizeof(T);
	return ::uncompress(reinterpret_cast<Bytef*>(&obj),
						&tmpSize,
						reinterpret_cast<const Bytef*>(_data),
						sizeof(_data));
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
