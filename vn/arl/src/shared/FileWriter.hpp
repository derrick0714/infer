#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include <boost/filesystem/path.hpp>

#include "Writer.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename T>
class FileWriter : public Writer<T> {
  public:
	/// \brief Constructor
	FileWriter()
		:_fileName()
	{
	}

	/// \brief Virtual destructor
	virtual ~FileWriter() {
	}

	/// \brief Open a file
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	virtual bool open(const boost::filesystem::path &fileName) = 0;

	/// \brief Close the currently open file
	/// \returns true if the file was perviously open, and was closed
	/// successfully
	/// \note a call to close on an already-closed FileWriter returns false
	virtual bool close() = 0;

  protected:
	/// \brief Write a blob_data object to the file
	/// \param obj the object to write
	/// \returns true if the write was successful
	virtual bool write(const T &obj, blob_data_tag) = 0;

	/// \brief Write a plain_old_data object to the file
	/// \param obj the object to write
	/// \returns true if the write was successful
	virtual bool write(const T &obj, plain_old_data_tag) = 0;

	/// \brief Write a serializable_data object to the file
	/// \param obj the object to write
	/// \returns true if the write was successful
	virtual bool write(const T &obj, serializable_data_tag) = 0;

	/// \brief The error status
	using Writer<T>::_error;

	/// \brief The error message
	using Writer<T>::_errorMsg;

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;
};	

} // namespace shared
} // namespace arl
} // namespace vn

#endif
