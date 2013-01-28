#ifndef BIN_CONNECTIONSEARCH_CONNECTIONSEARCHPOSTGRESQLTABLEFORMAT_HPP
#define BIN_CONNECTIONSEARCH_CONNECTIONSEARCHPOSTGRESQLTABLEFORMAT_HPP

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <libpq-fe.h>

// FIXME this method is not quite right, but no time now...

#include "Base64.h"
#include "PostgreSQLConnection.hpp"

template <typename T>
class PostgreSQLTableFormat;

template <>
class PostgreSQLTableFormat <ConnectionSearchHTTP> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "http_id"));
		_columns.push_back(std::make_pair("uint32", "start_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "start_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "client_ip"));
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("uint16", "client_port"));
		_columns.push_back(std::make_pair("uint16", "server_port"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchHTTP &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_id()) + "', "
				 "'" + lexical_cast<string>(item.start_time().seconds()) + "', "
				 "'" + lexical_cast<string>(item.start_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(item.end_time().seconds()) + "', "
				 "'" + lexical_cast<string>(item.end_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(item.client_ip()) + "', "
				 "'" + lexical_cast<string>(item.server_ip()) + "', "
				 "'" + lexical_cast<string>(item.client_port()) + "', "
				 "'" + lexical_cast<string>(item.server_port()) + "');");

		return PQexecParams(conn.connection(),
							s.c_str(),
							0,
							NULL,
							NULL,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchHTTPRequest> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "http_request_id"));
		_columns.push_back(std::make_pair("uint32", "time_seconds"));
		_columns.push_back(std::make_pair("uint32", "time_microseconds"));
		_columns.push_back(std::make_pair("text", "type"));
		_columns.push_back(std::make_pair("text", "uri"));
		_columns.push_back(std::make_pair("text", "version"));
		_columns.push_back(std::make_pair("text", "host"));
		_columns.push_back(std::make_pair("text", "user_agent"));
		_columns.push_back(std::make_pair("text", "referer"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchHTTPRequest &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		const char * values[6];

		values[0] = item.type().c_str();
		values[1] = item.uri().c_str();
		values[2] = item.version().c_str();
		values[3] = item.host().c_str();
		values[4] = item.user_agent().c_str();
		values[5] = item.referer().c_str();

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_request_id()) + "', "
				 "'" + lexical_cast<string>(item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(item.time().microseconds()) + "', "
				 "$1, $2, $3, $4, $5, $6);");

		return PQexecParams(conn.connection(),
							s.c_str(),
							6,
							NULL,
							values,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchHTTPResponse> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "http_response_id"));
		_columns.push_back(std::make_pair("uint32", "time_seconds"));
		_columns.push_back(std::make_pair("uint32", "time_microseconds"));
		_columns.push_back(std::make_pair("text", "version"));
		_columns.push_back(std::make_pair("text", "status"));
		_columns.push_back(std::make_pair("text", "response"));
		_columns.push_back(std::make_pair("text", "reason"));
		_columns.push_back(std::make_pair("text", "content_type"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchHTTPResponse &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		const char * values[5];

		values[0] = item.version().c_str();
		values[1] = item.status().c_str();
		values[2] = item.response().c_str();
		values[3] = item.reason().c_str();
		values[4] = item.content_type().c_str();

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_request_id()) + "', "
				 "'" + lexical_cast<string>(item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(item.time().microseconds()) + "', "
				 "$1, $2, $3, $4, $5);");

		return PQexecParams(conn.connection(),
							s.c_str(),
							5,
							NULL,
							values,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchConnection> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "connection_id"));
		_columns.push_back(std::make_pair("uint32", "start_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "start_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_microseconds"));
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "ip_a"));
		_columns.push_back(std::make_pair("uint32", "ip_b"));
		_columns.push_back(std::make_pair("uint16", "port_a"));
		_columns.push_back(std::make_pair("uint16", "port_b"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchConnection &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.index()) + "', "
				 "'" + lexical_cast<string>(
							item.start_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.start_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(
							item.end_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.end_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(
							(int) item.protocol()) + "', "
				 "'" + lexical_cast<string>(
							item.ip_a()) + "', "
				 "'" + lexical_cast<string>(
							item.ip_b()) + "', "
				 "'" + lexical_cast<string>(
							item.port_a()) + "', "
				 "'" + lexical_cast<string>(
							item.port_b()) + "');");

		return PQexecParams(conn.connection(),
							s.c_str(),
							0,
							NULL,
							NULL,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchNeoflow> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "neoflow_id"));
		_columns.push_back(std::make_pair("uint32", "start_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "start_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_microseconds"));
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "src_ip"));
		_columns.push_back(std::make_pair("uint32", "dst_ip"));
		_columns.push_back(std::make_pair("uint16", "src_port"));
		_columns.push_back(std::make_pair("uint16", "dst_port"));
		_columns.push_back(std::make_pair("uint32", "packet_count"));
		_columns.push_back(std::make_pair("uint32", "byte_count"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchNeoflow &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.index()) + "', "
				 "'" + lexical_cast<string>(
							item.start_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.start_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(
							item.end_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.end_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(
							(int) item.protocol()) + "', "
				 "'" + lexical_cast<string>(
							item.source_ip()) + "', "
				 "'" + lexical_cast<string>(
							item.destination_ip()) + "', "
				 "'" + lexical_cast<string>(
							item.source_port()) + "', "
				 "'" + lexical_cast<string>(
							item.destination_port()) + "', "
				 "'" + lexical_cast<string>(
							item.packet_count()) + "', "
				 "'" + lexical_cast<string>(
							item.byte_count()) + "');");

		return PQexecParams(conn.connection(),
							s.c_str(),
							0,
							NULL,
							NULL,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchDNS> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(6);

		_columns.push_back(std::make_pair("uint32", "dns_id"));
		_columns.push_back(std::make_pair("uint32", "query_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "query_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "response_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "response_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "client_ip"));
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("uint32", "client_port"));
		_columns.push_back(std::make_pair("uint32", "server_port"));
		_columns.push_back(std::make_pair("TEXT", "query_name"));
		_columns.push_back(std::make_pair("uint16", "query_type"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchDNS &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[1];

		values[0] = item.query_name().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.dns_index()) + "', "
				 "'" + lexical_cast<string>(
							item.query_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.query_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(
							item.response_time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.response_time().microseconds()) + "', "
				 "'" + lexical_cast<string>(item.client_ip()) + "', "
				 "'" + lexical_cast<string>(item.server_ip()) + "', "
				 "'" + lexical_cast<string>(item.client_port()) + "', "
				 "'" + lexical_cast<string>(item.server_port()) + "', "
				 "$1, "
				 "'" + lexical_cast<string>(item.query_type()) + "');");

		return PQexecParams(conn.connection(),
							s.c_str(),
							1,
							NULL,
							values,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <ConnectionSearchDNSResponse> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(5);

		_columns.push_back(std::make_pair("uint32", "dns_response_id"));
		_columns.push_back(std::make_pair("TEXT", "name"));
		_columns.push_back(std::make_pair("uint16", "type"));
		_columns.push_back(std::make_pair("bytea", "resource_data"));
		_columns.push_back(std::make_pair("int4", "ttl"));
	}

	bool create(PostgreSQLConnection &conn,
				const std::string &schema,
				const std::string &table)
	{
		if (!conn.isOpen()) {
			return false;
		}

		PGresult *result(
			PQexec(conn.connection(),
				   makeCreateString(schema, table).c_str()));

		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			PQclear(result);
			return false;
		}

		PQclear(result);
		return true;
	}

	std::string makeCreateString(const std::string &schema,
								 const std::string &table)
	{
		using namespace std;

		string s("CREATE TABLE \"" + schema + "\".\"" + table + "\" (");
		for (vector <pair <string, string> >::iterator i(_columns.begin());
			 i != _columns.end();
			 ++i)
		{
			s.append("\"" + i->second + "\" " + i->first + ", ");
		}
		s.replace(s.size() - 2, 2, ");");

		return s;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const ConnectionSearchDNSResponse &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[2];
		int lengths[2];
		size_t tmpLen;

		values[0] = item.name().c_str();
		lengths[0] = item.name().length();
		values[1] = reinterpret_cast <const char *>(
						PQescapeByteaConn(conn.connection(),
										  reinterpret_cast<const unsigned char *>(item.resource_data().data()),
										  item.resource_data().length(),
										  &tmpLen));
		lengths[1] = tmpLen;
		
		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.dns_index()) + "', "
				 "$1, "
				 "'" + lexical_cast<string>(item.type()) + "', "
				 "$2, "
				 "'" + lexical_cast<string>(item.ttl()) + "');");

		PGitem *ret(PQexecParams(conn.connection(),
							s.c_str(),
							2,
							NULL,
							values,
							lengths,
							NULL,
							0));
		PQfreemem(const_cast<void *>(reinterpret_cast<const void *>(values[1])));

		return ret;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

#endif
