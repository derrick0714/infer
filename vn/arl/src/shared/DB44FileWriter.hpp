#ifndef DB44FILEWRITER_HPP
#define DB44FILEWRITER_HPP

#include <db44/db_cxx.h>

#include "DataTypeTraits.hpp"
#include "FilesystemHelpers.h"

namespace vn {
namespace arl {
namespace shared {

/// \brief A BerkeleyDB-4.4 FileWriter
/// \todo open db with DB_CXX_NO_EXCEPTIONS and avoid exception overhead
///
/// This class provides synchronous write access to a BerkeleyDB-4.4 RECNO 
/// database.
template <typename T>
class DB44FileWriter {
  public:
	// this is a file_writer (template inheritance)
	typedef file_writer_type_tag category;
	typedef T value_type;


	/// \brief Constructor
	DB44FileWriter();

	/// \brief Destructor
	~DB44FileWriter();

	/// \brief Open a BerkeleyDB-4.4 file
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	bool open(const boost::filesystem::path &fileName);

	/// \brief Close the currently open BerkeleyDB-4.4 database file
	/// \returns true if the database was perviously open, and was closed
	/// successfully
	bool close();

	/// \brief Write an object
	/// \param obj the object to write
	bool write(const T& obj) {
		return write(obj, typename T::data_type());
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
		return _fileName;
	}

  private:
	/// \brief Disable copying
	DB44FileWriter(const DB44FileWriter &);

	/// \brief Disable copying
	DB44FileWriter & operator=(const DB44FileWriter &);

	/// \brief Write a blob_data object to the database file
	/// \param obj the object to write
	/// \returns true if the write was successful
	bool write(const T &obj, blob_data_tag);

	/// \brief Write a plain_old_data object to the database file
	/// \param obj the object to write
	/// \returns true if the write was successful
	bool write(const T &obj, plain_old_data_tag);

	/// \brief Write a serializable_data object to the database file
	/// \param obj the object to write
	/// \returns true if the write was successful
	bool write(const T &obj, serializable_data_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;

	/// A pointer the the current database
	Db *_db;

	/// Whether or not the database is currently open
	bool _isOpen;
};

template <typename T>
DB44FileWriter<T>::DB44FileWriter()
	:_error(false),
	 _errorMsg(),
	 _fileName(),
	 _db(NULL),
	 _isOpen(false)
{
}

template <typename T>
DB44FileWriter<T>::~DB44FileWriter() {
	if (_isOpen) {
		close();
	}
	if (_db != NULL) {
		// uh oh. db not closed successfully...
		// delete _db anyway. client should have called close()
		delete _db;
	}
}

template <typename T>
bool DB44FileWriter<T>::open(const boost::filesystem::path &fileName) {
	if (_isOpen) {
		_error = true;
		_errorMsg.assign("Database is already open.");
		return false;
	}

	using namespace boost::filesystem;
	
	if (fileName.parent_path() != path("")) {
		try {
			mkdir_p(fileName.parent_path());
		} catch (const filesystem_error &e) {
			_error = true;
			_errorMsg.assign("Unable to create directory '");
			_errorMsg.append(e.path1().file_string());
			_errorMsg.append("': ");
			_errorMsg.append(e.code().message());

			return false;
		}
	}

	_db = new Db(NULL, 0);

	try {
		_db -> open(NULL,
					fileName.file_string().c_str(),
					NULL,
					DB_RECNO,
					DB_CREATE,
					0644);
		_fileName = fileName;
		_isOpen = true;
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to open database '");
		_errorMsg.append(fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		delete _db;
	}

	return _isOpen;
}

template <typename T>
bool DB44FileWriter<T>::close() {
	if (!_isOpen) {
		_error = true;
		_errorMsg.append("Database not open.");
		return false;
	}

	try {
		_db -> close(0);
		delete _db;
		_db = NULL;
		_fileName = boost::filesystem::path();
		_isOpen = false;
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to cleanly close database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	return true;
}

template <typename T>
bool DB44FileWriter<T>::write(const T &obj, blob_data_tag)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.append("Database not open.");
		return false;
	}

	Dbt key;
	Dbt data;
	data.set_size(obj.size());
	data.set_data(const_cast<void *>(reinterpret_cast<const void *>(&obj)));

	try {
		_db -> put(NULL,
				   &key,
				   &data,
				   DB_APPEND);
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to append to database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	return true;
}

template <typename T>
bool DB44FileWriter<T>::write(const T &obj, plain_old_data_tag)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.append("Database not open.");
		return false;
	}

	Dbt key;
	Dbt data;
	data.set_size(sizeof(obj));
	data.set_data(const_cast<void *>(reinterpret_cast<const void *>(&obj)));

	try {
		_db -> put(NULL,
				   &key,
				   &data,
				   DB_APPEND);
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to append to database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	return true;
}

template <typename T>
bool DB44FileWriter<T>::write(const T &, serializable_data_tag)
{
	_error = true;
	_errorMsg.assign("write() not implemented yet for serializable data.");
	return false;
}

} // namespace shared
} // namespace arl
} // namespace shared

#endif
