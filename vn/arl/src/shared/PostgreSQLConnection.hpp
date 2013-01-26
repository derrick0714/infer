#ifndef POSTGRESQLCONNECTION_HPP
#define POSTGRESQLCONNECTION_HPP

#include <libpq-fe.h>
#include <string>

namespace vn {
namespace arl {
namespace shared {

class PostgreSQLConnection {
  public:
	PostgreSQLConnection()
		:_connection(NULL),
		 _host(),
		 _hostaddr(),
		 _port(),
		 _dbname(),
		 _user(),
		 _password(),
		 _connect_timeout(),
		 _options(),
		 _sslmode(),
		 _requiressl(),
		 _krbsrvname(),
		 _gsslib(),
		 _service(),
		 _error(false),
		 _errorMsg()
	{
	}

	~PostgreSQLConnection() {
		if (_connection != NULL) {
			PQfinish(_connection);
		}
	}

	bool isOpen() const {
		return _connection != NULL && PQstatus(_connection) == CONNECTION_OK;
	}

	bool open() {
		if (_connection != NULL) {
			PQreset(_connection);
			if (PQstatus(_connection) != CONNECTION_OK) {
				_error = true;
				_errorMsg.assign(PQerrorMessage(_connection));
				_errorMsg.resize(_errorMsg.size() - 1);
				return false;
			}

			return true;
		}

		std::string connStr;
		if (!_host.empty()) {
			connStr.append("host = '" + _host + "' ");
		}
		if (!_hostaddr.empty()) {
			connStr.append("hostaddr = '" + _hostaddr + "' ");
		}
		if (!_port.empty()) {
			connStr.append("port = '" + _port + "' ");
		}
		if (!_dbname.empty()) {
			connStr.append("dbname = '" + _dbname + "' ");
		}
		if (!_user.empty()) {
			connStr.append("user = '" + _user + "' ");
		}
		if (!_password.empty()) {
			connStr.append("password = '" + _password + "' ");
		}
		if (!_connect_timeout.empty()) {
			connStr.append("connect_timeout = '" + _connect_timeout + "' ");
		}
		if (!_options.empty()) {
			connStr.append("options = '" + _options + "' ");
		}
		if (!_sslmode.empty()) {
			connStr.append("sslmode = '" + _sslmode + "' ");
		}
		if (!_requiressl.empty()) {
			connStr.append("requiressl = '" + _requiressl + "' ");
		}
		if (!_service.empty()) {
			connStr.append("service = '" + _service + "' ");
		}

		_connection = PQconnectdb(connStr.c_str());
		if (PQstatus(_connection) != CONNECTION_OK) {
			_error = true;
			_errorMsg.assign(PQerrorMessage(_connection));
			_errorMsg.resize(_errorMsg.size() - 1);
			return false;
		}

		return true;
	}

	bool close() {
		if (!isOpen()) {
			_error = true;
			_errorMsg.assign("Connection not open.");
			return false;
		}

		PQfinish(_connection);
		_connection = NULL;

		return true;
	}

	std::string error() const {
		return _errorMsg;
	}

	operator bool() const {
		return !_error;
	}

	PGconn * connection() const {
		return _connection;
	}

	std::string host() const {
		return _host;
	}

	std::string hostaddr() const {
		return _hostaddr;
	}

	std::string port() const {
		return _port;
	}

	std::string dbname() const {
		return _dbname;
	}

	std::string user() const {
		return _user;
	}

	std::string password() const {
		return _password;
	}

	std::string connect_timeout() const {
		return _connect_timeout;
	}

	std::string options() const {
		return _options;
	}

	std::string sslmode() const {
		return _sslmode;
	}

	std::string requiressl() const {
		return _requiressl;
	}

	std::string krbsrvname() const {
		return _krbsrvname;
	}

	std::string gsslib() const {
		return _gsslib;
	}

	std::string service() const {
		return _service;
	}

	void host(const std::string &host) {
		_host = host;
	}

	void hostaddr(const std::string &hostaddr) {
		_hostaddr = hostaddr;
	}

	void port(const std::string &port) {
		_port = port;
	}

	void dbname(const std::string &dbname) {
		_dbname = dbname;
	}

	void user(const std::string &user) {
		_user = user;
	}

	void password(const std::string &password) {
		_password = password;
	}

	void connect_timeout(const std::string &connect_timeout) {
		_connect_timeout = connect_timeout;
	}

	void options(const std::string &options) {
		_options = options;
	}

	void sslmode(const std::string &sslmode) {
		_sslmode = sslmode;
	}

	void requiressl(const std::string &requiressl) {
		_requiressl = requiressl;
	}

	void krbsrvname(const std::string &krbsrvname) {
		_krbsrvname = krbsrvname;
	}

	void gsslib(const std::string &gsslib) {
		_gsslib = gsslib;
	}

	void service(const std::string &service) {
		_service = service;
	}

  private:
	// no copying
	PostgreSQLConnection(const PostgreSQLConnection &);
	PostgreSQLConnection & operator= (const PostgreSQLConnection &);

	PGconn *_connection;

	std::string _host;
	std::string _hostaddr;
	std::string _port;
	std::string _dbname;
	std::string _user;
	std::string _password;
	std::string _connect_timeout;
	std::string _options;
	std::string _sslmode;
	std::string _requiressl;
	std::string _krbsrvname;
	std::string _gsslib;
	std::string _service;

	bool _error;
	std::string _errorMsg;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
