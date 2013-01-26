#ifndef INFER_INCLUDE_FLATFILEWRITER_HPP_
#define INFER_INCLUDE_FLATFILEWRITER_HPP_

#include <fstream>
#include <boost/filesystem.hpp>

#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"
#include "FlatFileHeaderV00.hpp"

/// \brief A FlatFileWriter
///
/// This class provides synchronous write access of fixed-size records to a
/// flat file.
template <typename T>
class FlatFileWriter {
  public:
	/// \brief typedef for the category
	typedef file_writer_type_tag category;

	/// \brief typedef for the value type
	typedef T value_type;

	/// \brief Constructor
	FlatFileWriter();

	/// \brief Destructor
	~FlatFileWriter();

	/// \brief Find out if the file is open
	/// \returns true if the file is open
	bool isOpen() const {
		return _isOpen;
	}

	/// \brief Open a file
	/// \param fileName the file to open
	/// \returns E_SUCCESS if the file was successfully opened
	ErrorStatus open(const boost::filesystem::path &fileName);

	/// \brief Close the currently open BerkeleyDB-4.4 database file
	/// \returns E_SUCCESS if the database was closed successfully
	ErrorStatus close();

	/// \brief Write an object
	/// \param obj the object to write
	/// \returns E_SUCCESS if the write was successful
	template <typename TPointer>
	ErrorStatus write(const TPointer obj) {
		return write(*obj, typename data_type_traits<T>::data_type());
	}

	/// \brief Get the name of the currently opened file
	/// \returns the name of the currently opened file
	boost::filesystem::path fileName() const {
		return _fileName;
	}

  private:
	/// \brief Disable copying
	FlatFileWriter(const FlatFileWriter &);

	/// \brief Disable copying
	FlatFileWriter & operator=(const FlatFileWriter &);

	/// \brief write a plain_old_data object to the file
	/// \param obj the object to write
	/// \returns E_SUCCESS if the write was successful
	ErrorStatus write(const T &obj, plain_old_data_tag);

	/// \brief write blob_data object to the file
	/// \param obj the object to write
	/// \returns E_SUCCESS if the write was successful
	ErrorStatus write(const T &obj, blob_data_tag);

	/// \brief write serializable_data object to the file
	/// \param obj the object to write
	/// \returns E_SUCCESS if the write was successful
	ErrorStatus write(const T &obj, serializable_data_tag);

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;

	/// \brief The ofstream to use for writing to the file
	std::fstream _fileStream;

	/// \brief Whether or not the file is open for writing
	bool _isOpen;

	/// \brief The file header
	FlatFileHeaderV00 _flatFileHeaderV00;

	/// \brief The number of records written
	uint64_t _recordsWritten;
};

template <typename T>
FlatFileWriter<T>::FlatFileWriter()
	:_fileName(),
	 _fileStream(),
	 _isOpen(false),
	 _flatFileHeaderV00(),
	 _recordsWritten(0)
{
}

template <typename T>
FlatFileWriter<T>::~FlatFileWriter() {
	if (_isOpen) {
		close();
	}
}

template <typename T>
ErrorStatus FlatFileWriter<T>::open(const boost::filesystem::path &fileName)
{
	if (_isOpen) {
		return E_ISOPEN;
	}

	using namespace boost::filesystem;
	
	if (!exists(fileName.parent_path())) {
		return E_NOPARENT;
	} 
	
	bool existed(exists(fileName));

	ErrorStatus errorStatus;
	if (existed) {
		_fileStream.open(fileName.string().c_str(),
						 std::ios_base::in |
							std::ios_base::out |
							std::ios_base::binary);
		if (!_fileStream) {
			return E_FSTREAM;
		}

		errorStatus = _flatFileHeaderV00.unserialize(_fileStream);
		if (errorStatus != E_SUCCESS) {
			_fileStream.close();
			return errorStatus;
		}
		if (_flatFileHeaderV00.type() != data_type_traits<T>::TypeID()) {
			return E_TYPEMISMATCH;
		}

		// position write pointer
		// TODO implement index.
		_fileStream.seekp(0, std::ios_base::end);
		if (!_fileStream) {
			_fileStream.close();
			return E_FSTREAM;
		}
	} else {
		_fileStream.open(fileName.string().c_str(),
						 std::ios_base::out |
							std::ios_base::binary);
		if (!_fileStream) {
			return E_FSTREAM;
		}

		_flatFileHeaderV00.clear();
		_flatFileHeaderV00.type(data_type_traits<T>::TypeID());
		if ((data_type_traits<T>::TypeID() & 0x80) == 0) {
			_flatFileHeaderV00.recordSize(sizeof(T));
		}

		errorStatus = _flatFileHeaderV00.serialize(_fileStream);
		if (errorStatus != E_SUCCESS) {
			_fileStream.close();
			return errorStatus;
		}
	}

	_fileName = fileName;
	_isOpen = true;
	_recordsWritten = 0;

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileWriter<T>::close() {
	if (!_isOpen) {
		return E_NOTOPEN;
	}
	
	_fileName = boost::filesystem::path();
	_isOpen = false;

	_fileStream.flush();

	_fileStream.seekp(0, std::ios_base::beg);
	if (!_fileStream) {
		_fileStream.close();
		return E_FSTREAM;
	}

	_flatFileHeaderV00.recordCount(_recordsWritten +
								   _flatFileHeaderV00.recordCount());
	ErrorStatus errorStatus(_flatFileHeaderV00.serialize(_fileStream));
	if (errorStatus != E_SUCCESS) {
		_fileStream.close();
		return errorStatus;
	}

	_fileStream.close();
	if (!_fileStream) {
		return E_FSTREAM;
	}

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileWriter<T>::write(const T &obj, plain_old_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	_fileStream.write(reinterpret_cast<const char *>(&obj), sizeof(obj));
	if (!_fileStream) {
		return E_FSTREAM;
	}

	++_recordsWritten;

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileWriter<T>::write(const T &obj, blob_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	if (obj.size() > std::numeric_limits<uint16_t>::max()) {
		return E_TOOBIG;
	}

	static uint16_t size;
	size = static_cast<uint16_t>(obj.size());
	_fileStream.write(reinterpret_cast<const char *>(&size), sizeof(size));
	if (!_fileStream) {
		return E_FSTREAM;
	}

	_fileStream.write(reinterpret_cast<const char *>(&obj), size);
	if (!_fileStream) {
		return E_FSTREAM;
	}

	++_recordsWritten;

	return E_SUCCESS;
}

template <typename T>
ErrorStatus FlatFileWriter<T>::write(const T &obj, serializable_data_tag)
{
	if (!_isOpen) {
		return E_NOTOPEN;
	}

	obj.serialize(_fileStream);
	if (!_fileStream) {
		return E_FSTREAM;
	}

	++_recordsWritten;

	return E_SUCCESS;
}

#endif
