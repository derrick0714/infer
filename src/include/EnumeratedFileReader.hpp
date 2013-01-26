#ifndef INFER_INCLUDE_ENUMERATEDFILEREADER_HPP_
#define INFER_INCLUDE_ENUMERATEDFILEREADER_HPP_

#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

template <typename ReaderType, typename ReadEnumeratorType>
class EnumeratedFileReader {
  public:
	typedef reader_type_tag category;
	typedef typename ReaderType::value_type value_type;

	/// \brief Constructor
	EnumeratedFileReader();

	ErrorStatus init(boost::shared_ptr <ReadEnumeratorType> enumerator);
	
	/// \brief Destructor
	~EnumeratedFileReader();

	/// \brief Read an object
	/// \param obj the object to read
	/// \returns true if the object was read successfully, false otherwise.
	/// \note a return value of false does not indicate an error on it's own.
	/// Cast to bool to check for error. False here could just mean there's 
	/// nothing left to read.
	ErrorStatus read(value_type &obj) {
		return _read(obj, typename ReaderType::category());
	}

  private:
	/// \brief Read from a file reader
	/// \param obj the ob
	ErrorStatus _read(value_type &obj, file_reader_type_tag);

	/// \brief The currently opened file reader
	boost::shared_ptr <ReaderType> _reader;
	
	/// \brief The read enumerator to use for determining file names
	boost::shared_ptr <ReadEnumeratorType> _enumerator;

	/// \brief An itrator over the files to read from
	typename ReadEnumeratorType::iterator _file;
};

template <typename ReaderType, typename ReadEnumeratorType>
EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
EnumeratedFileReader()
	:_reader(new ReaderType), // shared_ptr deletes this when it loses scope
	 _enumerator(),
	 _file()
{
}

template <typename ReaderType, typename ReadEnumeratorType>
ErrorStatus EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
init(boost::shared_ptr <ReadEnumeratorType> enumerator)
{
	_enumerator = enumerator;

	_file = _enumerator->begin();
	if (_file == _enumerator -> end()) {
		return E_EOF;
	}

	return _reader -> open(*_file);
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
ErrorStatus EnumeratedFileReader<ReaderType, ReadEnumeratorType>::
_read(value_type &obj, file_reader_type_tag) {
	static ErrorStatus errorStatus;

	// is the reader open?
	if (!_reader -> isOpen()) {
		return E_NOTOPEN;
	}

	// call read()
	errorStatus = _reader -> read(obj);
	
	// does _reader have an error?
	if (errorStatus != E_EOF) {
		return errorStatus;
	}

	// _reader has nothing left to read. close it.
	if ((errorStatus = _reader->close()) != E_SUCCESS) {
		return errorStatus;
	}

	// time to get the next file
	++_file;
	if (_file == _enumerator -> end()) {
		// no more files. we're done here
		return E_EOF;
	}

	// open the next file
	if ((errorStatus = _reader -> open(*_file)) != E_SUCCESS) {
		return errorStatus;
	}

	// let's try that again
	return _read(obj, typename ReaderType::category());
}

#endif
