#ifndef ENUMERATEDFILEWRITER_HPP
#define ENUMERATEDFILEWRITER_HPP

#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \todo efficiently keep multiple files open, and figure out a clean way to
/// close old ones. Ie. have a maxOpenedFiles parameter, or maybe a timeout of
/// some sort. Maybe even have a different class for each of those. -Justin
template <typename WriterType, typename WriteEnumeratorType>
class EnumeratedFileWriter {
  public:
	typedef writer_type_tag category;
	typedef typename WriterType::value_type value_type;

	/// \brief Constructor
	explicit EnumeratedFileWriter
						(boost::shared_ptr <WriteEnumeratorType> enumerator);
	
	/// \brief Write an object
	/// \param obj the object to write
	bool write(const value_type &obj) {
		return _write(obj, typename WriterType::category());
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

	/// \brief Close the encapsulated file writer
	/// \returns true if the file writer closed successfully
	bool close() {
		if (!_writer -> close()) {
			_error = true;
			_errorMsg = _writer -> error();
		}

		return !_error;
	}

  private:
	bool _write(const value_type &obj, file_writer_type_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The currently opened file writer
	boost::shared_ptr <WriterType> _writer;
	
	/// \brief The write enumerator to use for determining file names
	boost::shared_ptr <WriteEnumeratorType> _enumerator;
};

template <typename WriterType, typename WriteEnumeratorType>
EnumeratedFileWriter<WriterType, WriteEnumeratorType>::
EnumeratedFileWriter(boost::shared_ptr <WriteEnumeratorType> enumerator)
	:_error(false),
	 _errorMsg(),
	 _writer(new WriterType), // shared_ptr deletes this when it loses scope
	 _enumerator(enumerator)
{
}

template <typename WriterType, typename WriteEnumeratorType>
bool EnumeratedFileWriter<WriterType, WriteEnumeratorType>::
_write(const value_type &obj, file_writer_type_tag) {
	boost::filesystem::path fileName(_enumerator->getFileName(obj));
	
	if (_writer->fileName() == fileName) {
		if (!_writer -> write(obj)) {
			_error = true;
			_errorMsg = _writer -> error();
		}

		return !_error;
	} 
	
	if (_writer->fileName() != "") {
		if (!_writer -> close()) {
			_error = true;
			_errorMsg = _writer -> error();

			return !_error;
		}
	}

	if (!_writer -> open(fileName)) {
		_error = true;
		_errorMsg = _writer -> error();

		return !_error;
	}

	if (!_writer -> write(obj)) {
		_error = true;
		_errorMsg = _writer -> error();
	}

	return !_error;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
