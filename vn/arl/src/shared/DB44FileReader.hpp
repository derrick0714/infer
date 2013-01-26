#ifndef DB44FILEREADER_HPP
#define DB44FILEREADER_HPP

#include <db44/db_cxx.h>
#include <boost/filesystem.hpp>

#include "DataTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A BerkeleyDB-4.4 FileReader
/// \todo open db with DB_CXX_NO_EXCEPTIONS and avoid exception overhead
///
/// This class provides synchronous read access to a BerkeleyDB-4.4 RECNO 
/// database.
template <typename T>
class DB44FileReader {
  public:
	// this is a file_reader (template inheritance)
	typedef file_reader_type_tag category;
	typedef T value_type;


	/// \brief Constructor
	DB44FileReader();

	/// \brief Destructor
	~DB44FileReader();

	/// \brief Open a BerkeleyDB-4.4 file
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	bool open(const boost::filesystem::path &fileName);

	/// \brief Find out if the db is open
	/// \returns true if the db is open
	bool isOpen() const {
		return _isOpen;
	}

	/// \brief Close the currently open BerkeleyDB-4.4 database file
	/// \returns true if the database was perviously open, and was closed
	/// successfully
	bool close();

	/// \brief Read an object
	/// \param obj the object to read
	bool read(T &obj) {
		return read(obj, typename data_type_traits<T>::data_type());
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
	DB44FileReader(const DB44FileReader &);

	/// \brief Disable copying
	DB44FileReader & operator=(const DB44FileReader &);

	/// \brief Read a blob_data object from the database file
	/// \param obj the object to read
	/// \returns true if the read was successful
	bool read(T &obj, blob_data_tag);

	/// \brief Read a plain_old_data object to the database file
	/// \param obj the object to read
	/// \returns true if the read was successful
	bool read(T &obj, plain_old_data_tag);

	/// \brief Read a serializable_data object to the database file
	/// \param obj the object to read
	/// \returns true if the read was successful
	bool read(T &obj, serializable_data_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;

	/// A pointer the the current database
	Db *_db;

	/// A pointer to the curser used to iterate over items in the db
	Dbc *_dbc;

	/// Whether or not the database is currently open
	bool _isOpen;
};

template <typename T>
DB44FileReader<T>::DB44FileReader()
	:_error(false),
	 _errorMsg(),
	 _fileName(),
	 _db(NULL),
	 _dbc(NULL),
	 _isOpen(false)
{
}

template <typename T>
DB44FileReader<T>::~DB44FileReader() {
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
bool DB44FileReader<T>::open(const boost::filesystem::path &fileName) {
	if (_isOpen) {
		_error = true;
		_errorMsg.assign("Database is already open.");
		return false;
	}

	using namespace boost::filesystem;
	
	_db = new Db(NULL, 0);

	try {
		_db -> open(NULL,
					fileName.file_string().c_str(),
					NULL,
					DB_RECNO,
					DB_RDONLY,
					0644);
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to open database '");
		_errorMsg.append(fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		delete _db;
	}

	try {
		_db -> cursor(NULL, &_dbc, 0);
		_fileName = fileName;
		_isOpen = true;
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to open cursor for database '");
		_errorMsg.append(fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		delete _db;
	}	

	return _isOpen;
}

template <typename T>
bool DB44FileReader<T>::close() {
	if (!_isOpen) {
		_error = true;
		_errorMsg.assign("Database not open.");
		return false;
	}

	try {
		_dbc -> close();
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to cleanly close cursor for database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

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
bool DB44FileReader<T>::read(T &obj, blob_data_tag)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.append("Database not open.");
		return false;
	}

	Dbt key;
	Dbt data;
	data.set_flags(DB_DBT_USERMEM);
	data.set_ulen(T::DataSize);
	data.set_data(reinterpret_cast<void *>(obj.data()));

	int ret;
	try {
		ret = _dbc -> get(&key,
						  &data,
						  DB_NEXT);
		obj.size(data.get_size());
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to read from database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	if (ret == DB_NOTFOUND) {
		return false;
	}

	return true;
}

template <typename T>
bool DB44FileReader<T>::read(T &obj, plain_old_data_tag)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.assign("Database not open.");
		return false;
	}

	Dbt key;
	Dbt data;
	data.set_flags(DB_DBT_USERMEM);
	data.set_ulen(sizeof(T));
	data.set_data(reinterpret_cast<void *>(&obj));

	int ret;
	try {
		ret = _dbc -> get(&key,
						  &data,
						  DB_NEXT);
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to read from database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	if (ret == DB_NOTFOUND) {
		return false;
	}

	return true;
}

template <typename T>
bool DB44FileReader<T>::read(T &obj, serializable_data_tag)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.assign("Database not open.");
		return false;
	}

	Dbt key;
	Dbt data;

	int ret;
	try {
		ret = _dbc -> get(&key,
						  &data,
						  DB_NEXT);
	} catch (const DbException &e) {
		_error = true;
		_errorMsg.assign("Unable to read from database '");
		_errorMsg.append(_fileName.file_string());
		_errorMsg.append("': ");
		_errorMsg.append(e.what());

		return false;
	}

	if (ret == DB_NOTFOUND) {
		return false;
	}

	if (!obj.unserialize(std::string(
								reinterpret_cast<char*>(data.get_data()),
														data.get_size())))
	{
		_error = true;
		_errorMsg.assign("Unable to unserialize object.");
		return false;
	}

	return true;
}

} // namespace shared
} // namespace arl
} // namespace shared

#endif
