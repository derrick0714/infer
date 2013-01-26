#ifndef ZLIBCOMPRESSEDFILEWRITER
#define ZLIBCOMPRESSEDFILEWRITER

#include <boost/shared_ptr.hpp>

#include "ZlibCompressedData.hpp"
#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename DataType, typename WriterType>
class ZlibCompressedFileWriter {
  public:
	typedef file_writer_type_tag category;
	typedef DataType value_type;

	/// \brief Constructor
	ZlibCompressedFileWriter();

	/// \brief Write an object
	/// \param obj the object to write
	bool write(const value_type &obj) {
		compressionBuffer.compress(obj);
		if (!writer -> write(compressionBuffer)) {
			_error = true;
			_errorMsg = writer -> error();
		}

		return !_error;
	}

	/// \brief Act like a FileWriter
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	bool open(const boost::filesystem::path &fileName) {
		return _open(fileName, typename WriterType::category());
	}

	/// \brief Close the underlying file writer
	/// \returns true if the file was perviously open, and was closed
	/// successfully
	bool close() {
		return _close(typename WriterType::category());
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
		return writer -> fileName();
	}

  private:
	bool _open(const boost::filesystem::path &fileName, file_writer_type_tag);

	bool _close(file_writer_type_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The underlying writer
	boost::shared_ptr <WriterType> writer;

	/// \brief The compression buffer
	ZlibCompressedData <value_type> compressionBuffer;
};

template <typename DataType, typename WriterType>
ZlibCompressedFileWriter<DataType, WriterType>::
ZlibCompressedFileWriter()
	:_error(false),
	 _errorMsg(),
	 writer(new WriterType()),
	 compressionBuffer()
{
}

template <typename DataType, typename WriterType>
bool ZlibCompressedFileWriter<DataType, WriterType>::
_open(const boost::filesystem::path &fileName, file_writer_type_tag) {
	if (!writer -> open(fileName)) {
		_error = true;
		_errorMsg = writer -> error();
	}

	return !_error;
}

template <typename DataType, typename WriterType>
bool ZlibCompressedFileWriter<DataType, WriterType>::
_close(file_writer_type_tag) {
	if (!writer -> close()) {
		_error = true;
		_errorMsg = writer -> error();
	}

	return !_error;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
