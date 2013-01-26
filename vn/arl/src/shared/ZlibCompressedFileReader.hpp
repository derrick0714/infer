#ifndef ZLIBCOMPRESSEDFILEREADER
#define ZLIBCOMPRESSEDFILEREADER

#include <boost/shared_ptr.hpp>

#include "ZlibCompressedData.hpp"
#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename DataType, typename ReaderType>
class ZlibCompressedFileReader {
  public:
	typedef file_reader_type_tag category;
	typedef DataType value_type;

	/// \brief Constructor
	ZlibCompressedFileReader();

	/// \brief Read an object
	/// \param obj the object to write
	bool read(value_type &obj) {
		bool ret(_reader -> read(compressionBuffer));
		if (ret) {
			compressionBuffer.uncompress(obj);
			return true;
		}
		
		if (!(*_reader)) {
			_error = true;
			_errorMsg = _reader -> error();
		}

		return false;
	}

	/// \brief Act like a FileReader
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	bool open(const boost::filesystem::path &fileName) {
		return _open(fileName, typename ReaderType::category());
	}

	/// \brief Close the underlying file reader
	/// \returns true if the file was perviously open, and was closed
	/// successfully
	bool close() {
		return _close(typename ReaderType::category());
	}

	/// \brief Get the error string
	/// \returns the error string.
	std::string error() const {
		return _errorMsg;
	}

	/// \brief Boolean cast operator
	/// \returns true if there is no error
	///
	/// This is the means by which clients will test for an error.
	operator bool() const {
		return !_error;
	}

	/// \brief Get the name of the currently opened file
	/// \returns the name of the currently opened file
	boost::filesystem::path fileName() const {
		return _reader -> fileName();
	}

	bool isOpen() const {
		return _reader -> isOpen();
	}

  private:
	bool _open(const boost::filesystem::path &fileName, file_reader_type_tag);

	bool _close(file_reader_type_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The underlying reader
	boost::shared_ptr <ReaderType> _reader;

	/// \brief The compression buffer
	ZlibCompressedData <value_type> compressionBuffer;
};

template <typename DataType, typename ReaderType>
ZlibCompressedFileReader<DataType, ReaderType>::
ZlibCompressedFileReader()
	:_error(false),
	 _errorMsg(),
	 _reader(new ReaderType()),
	 compressionBuffer()
{
}

template <typename DataType, typename ReaderType>
bool ZlibCompressedFileReader<DataType, ReaderType>::
_open(const boost::filesystem::path &fileName, file_reader_type_tag) {
	if (!_reader -> open(fileName)) {
		_error = true;
		_errorMsg = _reader -> error();
	}

	return !_error;
}

template <typename DataType, typename ReaderType>
bool ZlibCompressedFileReader<DataType, ReaderType>::
_close(file_reader_type_tag) {
	if (!_reader -> close()) {
		_error = true;
		_errorMsg = _reader -> error();
	}

	return !_error;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
