#ifndef ENUMERATEDFILEREADER_HPP
#define ENUMERATEDFILEREADER_HPP

#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename ReaderType, typename ReadEnumeratorType>
class EnumeratedFileReader {
  public:
	typedef reader_type_tag category;
	typedef typename ReaderType::value_type value_type;

	/// \brief Constructor
	explicit EnumeratedFileReader
						(boost::shared_ptr <ReadEnumeratorType> enumerator);
	
	/// \brief Destructor
	~EnumeratedFileReader();

	/// \brief Read an object
	/// \param obj the object to read
	/// \returns true if the object was read successfully, false otherwise.
	/// \note a return value of false does not indicate an error on it's own.
	/// Cast to bool to check for error. False here could just mean there's 
	/// nothing left to read.
	bool read(value_type &obj) {
		return _read(obj, typename ReaderType::category());
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

  private:
	/// \brief Read from a file reader
	/// \param obj the ob
	bool _read(value_type &obj, file_reader_type_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The currently opened file reader
	boost::shared_ptr <ReaderType> _reader;
	
	/// \brief The read enumerator to use for determining file names
	boost::shared_ptr <ReadEnumeratorType> _enumerator;

	/// \brief An itrator over the files to read from
	typename ReadEnumeratorType::iterator _file;
};

template <typename ReaderType, typename ReadEnumeratorType>
EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
EnumeratedFileReader(boost::shared_ptr <ReadEnumeratorType> enumerator)
	:_error(false),
	 _errorMsg(),
	 _reader(new ReaderType), // shared_ptr deletes this when it loses scope
	 _enumerator(enumerator),
	 _file(_enumerator -> begin())
{
	if (_file == _enumerator -> end()) {
		return;
	}

	if (!_reader -> open(*_file)) {
		_error = true;
		_errorMsg.assign(_reader -> error());
		return;
	}
}

template <typename ReaderType, typename ReadEnumeratorType>
EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
~EnumeratedFileReader() {
	if (_reader -> isOpen()) {
		// why would anyone be done with an EnumratedFileReader if they're not
		// done reading from it? Just in case...
		_reader -> close();
	}
}

template <typename ReaderType, typename ReadEnumeratorType>
bool EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
_read(value_type &obj, file_reader_type_tag) {
	// is the reader open?
	if (!_reader -> isOpen()) {
		return false;
	}

	// call read()
	if (_reader -> read(obj)) {
		// successful. we're done
		return true;
	}

	// does _reader have an error?
	if (!(*_reader)) {
		_error = true;
		_errorMsg.assign(_reader -> error());
		return false;
	}

	// _reader has nothing left to read. close it.
	if (!_reader -> close()) {
		_error = true;
		_errorMsg.assign(_reader -> error());
		return false;
	}

	// time to get the next file
	++_file;
	if (_file == _enumerator -> end()) {
		// no more files. we're done here
		return false;
	}

	// open the next file
	if (!_reader -> open(*_file)) {
		_error = true;
		_errorMsg.assign(_reader -> error());
		return false;
	}

	// let's try that again
	return _read(obj, typename ReaderType::category());
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
