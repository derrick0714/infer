#ifndef POSTGRESQLWRITER_HPP
#define POSTGRESQLWRITER_HPP

#include <libpq-fe.h>
#include <string>

#include "PostgreSQLConnection.hpp"
#include "PostgreSQLTableFormat.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename T>
class PostgreSQLWriter {
  public:
	typedef T value_type;

	explicit PostgreSQLWriter(PostgreSQLConnection &conn,
							  const std::string &schema,
							  const std::string &table);

	~PostgreSQLWriter();

	bool write(const T &obj);

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
};

template <typename T>
PostgreSQLWriter<T>::PostgreSQLWriter(PostgreSQLConnection &conn,
									  const std::string &schema,
									  const std::string &table)
	:_error(false),
	 _errorMsg(),
	 _conn(conn),
	 _schema(schema),
	 _table(table),
	 _tableFormat()
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
		_error = true;
		_errorMsg.assign("Schema \"");
		_errorMsg.append(_schema);
		_errorMsg.append("\" does not exist");
		return;
	}
	PQclear(result);

	// check table exists
	query.assign("SELECT \"table_name\" FROM \"information_schema\".\"tables\" "
				 "WHERE \"table_schema\" = '" + _schema + "'"
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
	if (PQntuples(result) == 0) {
		if (!_tableFormat.create(_conn, _schema, _table)) {
			PQclear(result);
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
	PQclear(result);
}

template <typename T>
PostgreSQLWriter<T>::~PostgreSQLWriter() {
	if (_conn.isOpen()) {
		_conn.close();
	}
}

template <typename T>
bool PostgreSQLWriter<T>::write(const T &obj) {
	// check db open, try to open if not
	if (!_conn.isOpen() && !_conn.open()) {
		_error = true;
		_errorMsg.assign("Unable to open connection to database: ");
		_errorMsg.append(_conn.error());
	}

	PGresult *result
				(_tableFormat.insert(_conn, _schema, _table, obj));

	ExecStatusType status(PQresultStatus(result));
	if (status != PGRES_COMMAND_OK) {
		PQclear(result);
		_error = true;
		_errorMsg.assign("Unable to write to table: ");
		_errorMsg.append(PQerrorMessage(_conn.connection()));
		_errorMsg.resize(_errorMsg.size() - 1);
		return false;
	}
	PQclear(result);

	return true;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
