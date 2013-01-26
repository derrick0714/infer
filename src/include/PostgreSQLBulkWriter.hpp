#ifndef INFER_INCLUDE_POSTGRESQLBULKWRITER_HPP_
#define INFER_INCLUDE_POSTGRESQLBULKWRITER_HPP_

#include <libpq-fe.h>
#include <string>

#include "PostgreSQLConnection.hpp"
#include "PostgreSQLTableFormat.hpp"

template <typename T>
class PostgreSQLBulkWriter {
  public:
	typedef T value_type;

	explicit PostgreSQLBulkWriter(PostgreSQLConnection &conn,
							  const std::string &schema,
							  const std::string &table,
							  bool drop_if_exists = false);

	~PostgreSQLBulkWriter();

	bool write(const T &obj);

	bool flush();

	bool close();

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
	bool _error;
	std::string _errorMsg;
	PostgreSQLConnection &_conn;
	std::string _schema;
	std::string _table;
	PostgreSQLTableFormat <T> _tableFormat;
	bool _closed;
	std::string _buffer;
};

template <typename T>
PostgreSQLBulkWriter<T>::PostgreSQLBulkWriter(PostgreSQLConnection &conn,
									  const std::string &schema,
									  const std::string &table,
									  bool drop_if_exists)
	:_error(false),
	 _errorMsg(),
	 _conn(conn),
	 _schema(schema),
	 _table(table),
	 _tableFormat(),
	 _closed(false),
	 _buffer()
{
	// check db open, try to open if not
	if (!_conn.isOpen() && !_conn.open()) {
		_error = true;
		_errorMsg.assign("Unable to open connection to database: ");
		_errorMsg.append(_conn.error());
		return;
	}

	// check schema exists
	std::string query("SELECT \"schema_name\" FROM \"information_schema\".\"schemata\" "
					  "WHERE \"schema_name\" = '" + _schema + "'");
	PGresult *result(PQexec(_conn.connection(), query.c_str()));
	ExecStatusType status(PQresultStatus(result));
	if (status != PGRES_TUPLES_OK) {
		PQclear(result);
		_error = true;
		_errorMsg.assign("Unable to check if schema exists: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return;
	}
	if (PQntuples(result) == 0) {
		PQclear(result);
		query = "CREATE SCHEMA \"" + _schema + "\";";
		result = PQexec(_conn.connection(), query.c_str());
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			_error = true;
			_errorMsg.assign("Unable to create schema \"");
			_errorMsg.append(_schema);
			_errorMsg.append("\"");
			return;
		}
	}
	PQclear(result);

	// check table exists
	query.assign("SELECT \"table_name\" FROM \"information_schema\".\"tables\" "
				 "WHERE \"table_schema\" = '" + _schema + "' "
				 "AND \"table_name\" = '" + _table + "'");
	result = PQexec(_conn.connection(), query.c_str());
	status = PQresultStatus(result);
	if (status != PGRES_TUPLES_OK) {
		PQclear(result);
		_error = true;
		_errorMsg.assign("Unable to check if table exists: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return;
	}
	if (PQntuples(result) == 0 || drop_if_exists) {
		PQclear(result);
		query.assign("DROP TABLE IF EXISTS \"" + _schema + "\".\"" + _table + "\"");
		result = PQexec(_conn.connection(), query.c_str());
		status = PQresultStatus(result);
		if (status != PGRES_COMMAND_OK) {
			PQclear(result);
			_error = true;
			_errorMsg.assign("Unable to drop table \"" + _schema + "\".\"" +
							 _table + "\": ");
			_errorMsg.append(PQerrorMessage(_conn.connection()));
			_errorMsg.resize(_errorMsg.size() - 1);
			return;
		}
		PQclear(result);
		if (!_tableFormat.create(_conn, _schema, _table)) {
			_error = true;
			_errorMsg.assign("Table \"");
			_errorMsg.append(_schema);
			_errorMsg.append("\".\"");
			_errorMsg.append(_table);
			_errorMsg.append("\" does not exist and could not be created: ");
			_errorMsg.append(PQerrorMessage(_conn.connection()));
			_errorMsg.resize(_errorMsg.size() - 1);
			return;
		}
	}
	else {
		PQclear(result);
	}

	// begin copy data
	query.assign("COPY \"" + _schema + "\".\"" + _table + "\" FROM STDIN");
	result = PQexec(_conn.connection(), query.c_str());
	status = PQresultStatus(result);
	if (status != PGRES_COPY_IN) {
		PQclear(result);
		_error = true;
		_errorMsg.assign("COPY command failed: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return;
	}
	PQclear(result);
}

template <typename T>
PostgreSQLBulkWriter<T>::~PostgreSQLBulkWriter() {
	if (!_closed) {
		close();
	}
}

template <typename T>
bool PostgreSQLBulkWriter<T>::write(const T &obj) {
	// check db open
	if (!_conn.isOpen()) {
		_error = true;
		_errorMsg.assign("Connection not open.");
		return false;
	}

	_tableFormat.copy_text(obj, _buffer);

	return true;
}

template <typename T>
bool PostgreSQLBulkWriter<T>::flush() {
	if (_buffer.empty()) {
		return true;
	}

	// check db open
	if (!_conn.isOpen()) {
		_error = true;
		_errorMsg.assign("Connection not open.");
		return false;
	}

	if (PQputCopyData(_conn.connection(),
					  _buffer.data(),
					  _buffer.size()) != 1)
	{
		_error = true;
		_errorMsg.assign("Unable to COPY buffer: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return false;
	}

	_buffer.clear();

	return true;
}

template <typename T>
bool PostgreSQLBulkWriter<T>::close() {
	// check db open
	if (!_conn.isOpen()) {
		_error = true;
		_errorMsg.assign("Connection not open.");
		return false;
	}

	if (!flush()) {
		return false;
	}

/*
	if (PQputCopyData(_conn.connection(),
					  "\377\377",
					  2) != 1)
	{
		_error = true;
		_errorMsg.assign("Unable to COPY footer: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return false;
	}
*/
	if (PQputCopyEnd(_conn.connection(), NULL) != 1) {
		_error = true;
		_errorMsg.assign("PQPutCopyEnd() failed: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return false;
	}

	PGresult *result(PQgetResult(_conn.connection()));
	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
		PQclear(result);
		_error = true;
		_errorMsg.assign("COPY failed: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return false;
	}
	PQclear(result);

	_closed = true;
	return true;
}

#endif
