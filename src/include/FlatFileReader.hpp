#ifndef INFER_INCLUDE_FLATFILEREADER_HPP_
#define INFER_INCLUDE_FLATFILEREADER_HPP_

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"
#include "FlatFileHeaderV00.hpp"

/// \brief A FlatFileReader
///
/// This class provides synchronous read access of fixed-size records to a
/// flat file.
template <typename T>
class FlatFileReader {
  public:
	/// \brief typedef for the category
	typedef file_reader_type_tag category;

	/// \brief typedef for the value type
	typedef T value_type;

	/// \brief Constructor
	FlatFileReader();

	/// \brief Destructor
	~FlatFileReader();

	/// \brief Find out if the file is open
	/// \returns true if the file is open
	bool isOpen() const {
		return _isOpen;
	}

	/// \brief Open a file
	/// \param fileName the file to open
	/// \returns E_SUCCESS if the file was successfully opened
	ErrorStatus open(const boost::filesystem::path &fileName);

	/// \brief Close the currently open file
	/// \returns E_SUCCESS if the file was closed successfully
	ErrorStatus close();

	/// \brief Read the next object
	/// \param obj the object to read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj) {
		return read(obj, typename data_type_traits<T>::data_type());
	}

	/// \brief Read a specific object
	/// \param obj the object to read
	/// \param record_number the record number of the object to be read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj, size_t record_number) {
		return read(obj,
					record_number,
					typename data_type_traits<T>::data_type());
	}

	/// \brief Get the name of the currently opened file
	/// \returns the name of the currently opened file
	boost::filesystem::path fileName() const {
		return _fileName;
	}

  private:
	/// \brief Disable copying
	FlatFileReader(const FlatFileReader &);

	/// \brief Disable copying
	FlatFileReader & operator=(const FlatFileReader &);

	/// \brief Read a plain_old_data object from the file
	/// \param obj the object to read
	/// \param record_number the record number of the object to read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj, size_t record_number, plain_old_data_tag);

	/// \brief Read a plain_old_data object from the file
	/// \param obj the object to read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj, plain_old_data_tag);

	/// \brief Read a blob_data object to the database file
	/// \param obj the object to read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj, blob_data_tag);

	/// \brief Read a serializable_data object to the database file
	/// \param obj the object to read
	/// \returns E_SUCCESS if the read was successful
	ErrorStatus read(T &obj, serializable_data_tag);

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;

	/// A pointer the ifstream to use for reading from the file
	std::ifstream _fileStream;

	/// Whether or not the file is currently open
	bool _isOpen;

	/// \brief The file header
	FlatFileHeaderV00 _flatFileHeaderV00;

	/// \brief The current record
	uint64_t _curRecord;
};

template <typename T>
FlatFileReader<T>::FlatFileReader()
	:_fileName(),
	 _fileStream(),
	 _isOpen(false),
	 _curRecord(0)
{
}

template <typename T>
FlatFileReader<T>::~FlatFileReader() {
	if (_isOpen) {
		close();
	}
}

template <typename T>
ErrorStatus FlatFileReader<T>::open(const boost::filesystem::path &fileName) {
	if (_isOpen) {
		return E_ISOPEN;
	}

	using namespace boost::filesystem;

	if (!exists(fileName)) {
		return E_NOENT;
	}

	_fileStream.open(fileName.string().c_str());
	if (!_fileStream) {
		return E_FSTREAM;
	}

	ErrorStatus errorStatus(_flatFileHeaderV00.unserialize(_fileStream));
	if (errorStatus != E_SUCCESS) {
		_fileStream.close();
		return errorStatus;
	}
	if (_flatFileHeaderV00.type() != data_type_traits<T>::TypeID()) {
		return E_TYPEMISMATCH;
	}

	_fileName = fileName;
	_isOpen = true;
	_curRecord = 0;

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileReader<T>::close() {
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	_fileName = boost::filesystem::path();
	_isOpen = false;

	_fileStream.close();
	if (!_fileStream) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileReader<T>::read(T &obj,
									size_t record_number,
									plain_old_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	if (record_number >= _flatFileHeaderV00.recordCount()) {
		// a record past the end was requested. position the input stream
		// and set _curRecord to the EOF
		_curRecord = _flatFileHeaderV00.recordCount();
		_fileStream.seekg(std::ios::end);
		return E_EOF;
	}

	// just position the get pointer and set _curRecord, then call read()
	_fileStream.seekg((boost::numeric_cast<int>(record_number)
						- boost::numeric_cast<int>(_curRecord))
							* sizeof(obj),
					  std::ios_base::cur);
	_curRecord = record_number;

	return read(obj, plain_old_data_tag());
}

template <typename T>
ErrorStatus FlatFileReader<T>::read(T &obj, plain_old_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	if (_curRecord >= _flatFileHeaderV00.recordCount()) {
		return E_EOF;
	}

	_fileStream.read(reinterpret_cast<char *>(&obj), sizeof(obj));
	if (!_fileStream) {
		if (_fileStream.eof()) {
			_fileStream.clear();
			return E_EOF;
		}
		return E_FSTREAM;
	}

	++_curRecord;
	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileReader<T>::read(T &obj, blob_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	if (_curRecord >= _flatFileHeaderV00.recordCount()) {
		return E_EOF;
	}

	static uint16_t size;
	_fileStream.read(reinterpret_cast<char *>(&size), sizeof(size));
	if (!_fileStream) {
		if (_fileStream.eof()) {
			_fileStream.clear();
			return E_EOF;
		}
		return E_FSTREAM;
	}

	_fileStream.read(reinterpret_cast<char *>(&obj), size);
	if (!_fileStream) {
		if (_fileStream.eof()) {
			_fileStream.clear();
			return E_EOF;
		}
		return E_FSTREAM;
	}
	obj.size(size);

	++_curRecord;
	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileReader<T>::read(T &obj, serializable_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	if (_curRecord >= _flatFileHeaderV00.recordCount()) {
		return E_EOF;
	}

	obj.unserialize(_fileStream);
	if (!_fileStream) {
		if (_fileStream.eof()) {
			_fileStream.clear();
			return E_EOF;
		}
		return E_FSTREAM;
	}

	++_curRecord;
	return E_SUCCESS;
}

#endif
