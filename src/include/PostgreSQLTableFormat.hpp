#ifndef POSTGRESQLTABLEFORMAT_HPP
#define POSTGRESQLTABLEFORMAT_HPP

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <libpq-fe.h>

// FIXME this method is not quite right, but no time now...

#include "HBFHTTPResult.hpp"
#include "../bin/search/SearchNeoflowResult.hpp"
#include "../bin/search/SearchHTTPResult.hpp"
#include "../bin/search/SearchDNSResult.hpp"
#include "../bin/search/SearchDNSResponseResult.hpp"
#include "../bin/connection_search/ConnectionSearchHTTP.hpp"
#include "../bin/connection_search/ConnectionSearchHTTPRequest.hpp"
#include "../bin/connection_search/ConnectionSearchHTTPResponse.hpp"
#include "../bin/connection_search/ConnectionSearchHTTPRequestReference.hpp"
#include "../bin/connection_search/ConnectionSearchHTTPResponseReference.hpp"
#include "../bin/connection_search/ConnectionSearchConnection.hpp"
#include "../bin/connection_search/ConnectionSearchNeoflow.hpp"
#include "../bin/connection_search/ConnectionSearchNeoflowReference.hpp"
#include "../bin/connection_search/ConnectionSearchConnectionHTTPReference.hpp"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "host_ip_index.hpp"
#include "host_index_pair.hpp"
#include "../lib/analysis/bandwidth_utilization/bandwidth_utilization_datum.hpp"
#include "../lib/analysis/host_bandwidth_utilization/host_bandwidth_utilization_datum.hpp"
#include "../lib/analysis/bandwidth_content/bandwidth_content_datum.hpp"
#include "../lib/analysis/host_bandwidth_content/host_bandwidth_content_datum.hpp"
#include "../lib/analysis/network_exposure/as_exposure_datum.hpp"
#include "../lib/analysis/host_exposure/host_as_exposure_datum.hpp"
#include "../bin/browser_stats/browser_stat.hpp"
#include "../bin/browser_stats/browser_version_stat.hpp"
#include "../bin/dns2psql/dns_row.hpp"
#include "../bin/dns2psql/dns_response_row.hpp"
#include "../bin/web_server_top_urls/top_url_datum.hpp"
#include "../bin/web_server_top_hosts/top_host_datum.hpp"
#include "../bin/web_server_browsers/web_server_browser_datum.hpp"
#include "../bin/web_server_crawlers/web_server_crawler_datum.hpp"
#include "../bin/web_server_servers/web_server_server_datum.hpp"

#include "network_stats.hpp"
#include "Base64.h"
#include "PostgreSQLConnection.hpp"

template <typename T>
class PostgreSQLTableFormat;

template <>
class PostgreSQLTableFormat <HBFHTTPResult> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(21);

		// HBF portion
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "sourceIP"));
		_columns.push_back(std::make_pair("uint32", "destinationIP"));
		_columns.push_back(std::make_pair("uint16", "sourcePort"));
		_columns.push_back(std::make_pair("uint16", "destinationPort"));
		_columns.push_back(std::make_pair("uint32", "startTime"));
		_columns.push_back(std::make_pair("uint32", "endTime"));
		_columns.push_back(std::make_pair("smallint", "country"));
		_columns.push_back(std::make_pair("uint16", "asn"));
		_columns.push_back(std::make_pair("TEXT", "matchingString"));

		// HTTP portion:

		// requests
		_columns.push_back(std::make_pair("TEXT", "httpRequestVersion"));
		_columns.push_back(std::make_pair("TEXT", "httpRequestType"));
		_columns.push_back(std::make_pair("TEXT", "httpURI"));
		_columns.push_back(std::make_pair("TEXT", "httpHost"));
		_columns.push_back(std::make_pair("TEXT", "httpUserAgent"));
		_columns.push_back(std::make_pair("TEXT", "httpReferer"));

		// responses
		_columns.push_back(std::make_pair("TEXT", "httpResponseVersion"));
		_columns.push_back(std::make_pair("TEXT", "httpStatus"));
		_columns.push_back(std::make_pair("TEXT", "httpResponse"));
		_columns.push_back(std::make_pair("TEXT", "httpReason"));
		_columns.push_back(std::make_pair("TEXT", "httpContentType"));
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
					  const HBFHTTPResult &result)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[11];

		if (result.http().request().time() == TimeStamp()) {
			values[0] = NULL; // httpRequestVersion
			values[1] = NULL; // httpRequestType
			values[2] = NULL; // httpURI
			values[3] = NULL; // httpHost
			values[4] = NULL; // httpUserAgent
			values[5] = NULL; // httpReferer
		} else {
			values[0] = result.http().request().version().c_str();
			values[1] = result.http().request().type().c_str();
			values[2] = result.http().request().uri().c_str();
			values[3] = result.http().request().host().c_str();
			values[4] = result.http().request().user_agent().c_str();
			values[5] = result.http().request().referer().c_str();
		}
		if (result.http().response().time() == TimeStamp()) {
			values[6]  = NULL; // httpResponseVersion
			values[7]  = NULL; // httpStatus
			values[8]  = NULL; // httpResponse
			values[9]  = NULL; // httpReason
			values[10] = NULL; // httpContentType
		} else {
			values[6]  = result.http().response().version().c_str();
			values[7]  = result.http().response().status().c_str();
			values[8]  = result.http().response().response().c_str();
			values[9]  = result.http().response().reason().c_str();
			values[10] = result.http().response().content_type().c_str();
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							(int) result.hbf().protocol()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().sourceIP()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().destinationIP()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().sourcePort()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().destinationPort()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().startTime().seconds()) + "', "
				 "'" + lexical_cast<string>(
							result.hbf().endTime().seconds()) + "', "
				 "NULL, " // country
				 "NULL, " // asn
				 "'" + Base64::encode(result.hbf().match().data(),
									  result.hbf().match().length()) + "', "
				 "$1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11);");

		return PQexecParams(conn.connection(),
							s.c_str(),
							11,
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
class PostgreSQLTableFormat <SearchHTTPResult> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(12);

		_columns.push_back(std::make_pair("uint32", "neoflow_index"));

		// requests
		_columns.push_back(std::make_pair("TEXT", "request_version"));
		_columns.push_back(std::make_pair("TEXT", "request_type"));
		_columns.push_back(std::make_pair("TEXT", "request_uri"));
		_columns.push_back(std::make_pair("TEXT", "request_host"));
		_columns.push_back(std::make_pair("TEXT", "request_useragent"));
		_columns.push_back(std::make_pair("TEXT", "request_referer"));

		// responses
		_columns.push_back(std::make_pair("TEXT", "response_version"));
		_columns.push_back(std::make_pair("TEXT", "response_status"));
		_columns.push_back(std::make_pair("TEXT", "response_response"));
		_columns.push_back(std::make_pair("TEXT", "response_reason"));
		_columns.push_back(std::make_pair("TEXT", "response_contenttype"));
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
					  const SearchHTTPResult &result)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[11];

		if (result.request().time() == TimeStamp()) {
			values[0] = NULL; // httpRequestVersion
			values[1] = NULL; // httpRequestType
			values[2] = NULL; // httpURI
			values[3] = NULL; // httpHost
			values[4] = NULL; // httpUserAgent
			values[5] = NULL; // httpReferer
		} else {
			values[0] = result.request().version().c_str();
			values[1] = result.request().type().c_str();
			values[2] = result.request().uri().c_str();
			values[3] = result.request().host().c_str();
			values[4] = result.request().user_agent().c_str();
			values[5] = result.request().referer().c_str();
		}
		if (result.response().time() == TimeStamp()) {
			values[6]  = NULL; // httpResponseVersion
			values[7]  = NULL; // httpStatus
			values[8]  = NULL; // httpResponse
			values[9]  = NULL; // httpReason
			values[10] = NULL; // httpContentType
		} else {
			values[6]  = result.response().version().c_str();
			values[7]  = result.response().status().c_str();
			values[8]  = result.response().response().c_str();
			values[9]  = result.response().reason().c_str();
			values[10] = result.response().content_type().c_str();
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(result.neoflow_index()) + "', "
				 "$1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11);");

		return PQexecParams(conn.connection(),
							s.c_str(),
							11,
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
class PostgreSQLTableFormat <SearchNeoflowResult> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(10);

		// HBF portion
		_columns.push_back(std::make_pair("uint32", "index"));
		_columns.push_back(std::make_pair("uint32", "start_time"));
		_columns.push_back(std::make_pair("uint32", "end_time"));
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
					  const SearchNeoflowResult &result)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							result.index()) + "', "
				 "'" + lexical_cast<string>(
							result.startTime().seconds()) + "', "
				 "'" + lexical_cast<string>(
							result.endTime().seconds()) + "', "
				 "'" + lexical_cast<string>(
							(int) result.protocol()) + "', "
				 "'" + lexical_cast<string>(
							result.sourceIP()) + "', "
				 "'" + lexical_cast<string>(
							result.destinationIP()) + "', "
				 "'" + lexical_cast<string>(
							result.sourcePort()) + "', "
				 "'" + lexical_cast<string>(
							result.destinationPort()) + "', "
				 "'" + lexical_cast<string>(
							result.numPackets()) + "', "
				 "'" + lexical_cast<string>(
							result.numBytes()) + "');");

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
class PostgreSQLTableFormat <SearchDNSResult> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(8);

		_columns.push_back(std::make_pair("uint32", "index"));
		_columns.push_back(std::make_pair("uint32", "neoflow_index"));
		_columns.push_back(std::make_pair("uint32", "query_time"));
		_columns.push_back(std::make_pair("uint32", "response_time"));
		_columns.push_back(std::make_pair("uint32", "client_ip"));
		_columns.push_back(std::make_pair("uint32", "server_ip"));
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
					  const SearchDNSResult &result)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[1];

		values[0] = result.queryName().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(result.index()) + "', "
				 "'" + lexical_cast<string>(result.neoflow_index()) + "', "
				 "'" + lexical_cast<string>(
							result.queryTime().seconds()) + "', "
				 "'" + lexical_cast<string>(
							result.responseTime().seconds()) + "', "
				 "'" + lexical_cast<string>(result.client_ip()) + "', "
				 "'" + lexical_cast<string>(result.server_ip()) + "', "
				 "$1, "
				 "'" + lexical_cast<string>(result.queryType()) + "');");

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
class PostgreSQLTableFormat <SearchDNSResponseResult> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.reserve(5);

		_columns.push_back(std::make_pair("uint32", "dns_index"));
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
					  const SearchDNSResponseResult &result)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[2];
		int lengths[2];
		size_t tmpLen;

		values[0] = result.name().c_str();
		lengths[0] = result.name().length();
		values[1] = reinterpret_cast <const char *>(
						PQescapeByteaConn(conn.connection(),
										  reinterpret_cast<const unsigned char *>(result.resource_data().data()),
										  result.resource_data().length(),
										  &tmpLen));

		lengths[1] = tmpLen;

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(result.dns_index()) + "', "
				 "$1, "
				 "'" + lexical_cast<string>(result.type()) + "', "
				 "$2, "
				 "'" + lexical_cast<string>(result.ttl()) + "');");

		PGresult *ret(PQexecParams(conn.connection(),
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
		_columns.push_back(std::make_pair("uint16", "protocol"));
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
							item.client_ip()) + "', "
				 "'" + lexical_cast<string>(
							item.server_ip()) + "', "
				 "'" + lexical_cast<string>(
							item.client_port()) + "', "
				 "'" + lexical_cast<string>(
							item.server_port()) + "');");

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
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "source_ip"));
		_columns.push_back(std::make_pair("uint32", "destination_ip"));
		_columns.push_back(std::make_pair("uint16", "source_port"));
		_columns.push_back(std::make_pair("uint16", "destination_port"));

		// requests
		_columns.push_back(std::make_pair("TEXT", "type"));
		_columns.push_back(std::make_pair("TEXT", "uri"));
		_columns.push_back(std::make_pair("TEXT", "version"));
		_columns.push_back(std::make_pair("TEXT", "host"));
		_columns.push_back(std::make_pair("TEXT", "user_agent"));
		_columns.push_back(std::make_pair("TEXT", "referer"));
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

		const char * values[6];

		values[0] = item.type().c_str();
		values[1] = item.uri().c_str();
		values[2] = item.version().c_str();
		values[3] = item.host().c_str();
		values[4] = item.user_agent().c_str();
		values[5] = item.referer().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_request_id()) + "', "
				 "'" + lexical_cast<string>(
							item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.time().microseconds()) + "', "
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
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "source_ip"));
		_columns.push_back(std::make_pair("uint32", "destination_ip"));
		_columns.push_back(std::make_pair("uint16", "source_port"));
		_columns.push_back(std::make_pair("uint16", "destination_port"));

		_columns.push_back(std::make_pair("TEXT", "version"));
		_columns.push_back(std::make_pair("TEXT", "status"));
		_columns.push_back(std::make_pair("TEXT", "reason"));
		_columns.push_back(std::make_pair("TEXT", "response"));
		_columns.push_back(std::make_pair("TEXT", "content_type"));
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

		const char * values[5];

		values[0] = item.version().c_str();
		values[1] = item.status().c_str();
		values[2] = item.reason().c_str();
		values[3] = item.response().c_str();
		values[4] = item.content_type().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_response_id()) + "', "
				 "'" + lexical_cast<string>(
							item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.time().microseconds()) + "', "
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
class PostgreSQLTableFormat <ConnectionSearchHTTPRequestReference> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "http_id"));
		_columns.push_back(std::make_pair("uint32", "http_request_id"));
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
					  const ConnectionSearchHTTPRequestReference &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_id()) + "', "
				 "'" + lexical_cast<string>(item.http_request_id()) + "');");

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
class PostgreSQLTableFormat <ConnectionSearchHTTPResponseReference> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "http_id"));
		_columns.push_back(std::make_pair("uint32", "http_response_id"));
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
					  const ConnectionSearchHTTPResponseReference &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.http_id()) + "', "
				 "'" + lexical_cast<string>(item.http_response_id()) + "');");

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
				 "'" + lexical_cast<string>(item.connection_id()) + "', "
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
		_columns.reserve(10);

		// HBF portion
		_columns.push_back(std::make_pair("uint32", "neoflow_id"));
		_columns.push_back(std::make_pair("uint32", "start_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "start_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "end_time_microseconds"));
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "source_ip"));
		_columns.push_back(std::make_pair("uint32", "destination_ip"));
		_columns.push_back(std::make_pair("uint16", "source_port"));
		_columns.push_back(std::make_pair("uint16", "destination_port"));
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
							item.neoflow_id()) + "', "
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
class PostgreSQLTableFormat <ConnectionSearchNeoflowReference> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "connection_id"));
		_columns.push_back(std::make_pair("uint32", "neoflow_id"));
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
					  const ConnectionSearchNeoflowReference &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.connection_id()) + "', "
				 "'" + lexical_cast<string>(item.neoflow_id()) + "');");

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
class PostgreSQLTableFormat <ConnectionSearchConnectionHTTPReference> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "connection_id"));
		_columns.push_back(std::make_pair("uint32", "http_id"));
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
					  const ConnectionSearchConnectionHTTPReference &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(item.connection_id()) + "', "
				 "'" + lexical_cast<string>(item.http_id()) + "');");

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
class PostgreSQLTableFormat <HTTPRequest> {
  public:
	PostgreSQLTableFormat()
		:_columns(),
		 _binary_tuples()
	{
		_columns.push_back(std::make_pair("uint32", "time_seconds"));
		_columns.push_back(std::make_pair("uint32", "time_microseconds"));
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "source_ip"));
		_columns.push_back(std::make_pair("uint32", "destination_ip"));
		_columns.push_back(std::make_pair("uint16", "source_port"));
		_columns.push_back(std::make_pair("uint16", "destination_port"));

		// requests
		_columns.push_back(std::make_pair("TEXT", "type"));
		_columns.push_back(std::make_pair("TEXT", "uri"));
		_columns.push_back(std::make_pair("TEXT", "version"));
		_columns.push_back(std::make_pair("TEXT", "host"));
		_columns.push_back(std::make_pair("TEXT", "user_agent"));
		_columns.push_back(std::make_pair("TEXT", "referer"));

		_binary_tuples = _columns.size();
		_binary_tuples = htons(_binary_tuples);
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

	bool copy_text(const HTTPRequest &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.time().seconds()) + '\t' +
					  lexical_cast<string>(item.time().microseconds()) + '\t' +
					  lexical_cast<string>((int) item.protocol()) + '\t' +
					  lexical_cast<string>(item.source_ip()) + '\t' +
					  lexical_cast<string>(item.destination_ip()) + '\t' +
					  lexical_cast<string>(item.source_port()) + '\t' +
					  lexical_cast<string>(item.destination_port()) + '\t');

		regex search("(\\\\)|([\\b])|(\\f)|(\\n)|(\\r)|(\\t)|(\\v)");
		string replace("(?1\\\\\\\\)(?2\\\\b)(?3\\\\f)"
					   "(?4\\\\n)(?5\\\\r)(?6\\\\t)(?7\\\\v)");

		string str(item.type());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.uri());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.version());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.host());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.user_agent());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.referer());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\n");

		return true;
	}

	bool copy_binary(const HTTPRequest &item,
					 std::string &buffer)
	{
		static const uint32_t be_4(htonl((uint32_t) 4));
		static const uint32_t be_2(htonl((uint32_t) 2));
		uint32_t tmp32;
		uint16_t tmp16;

		buffer.append(reinterpret_cast<const char *>(&_binary_tuples), 2);

		buffer.append(reinterpret_cast<const char *>(&be_4), 4);
		tmp32 = htonl(item.time().seconds());
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);

		buffer.append(reinterpret_cast<const char *>(&be_4), 4);
		tmp32 = htonl(item.time().microseconds());
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);

		buffer.append(reinterpret_cast<const char *>(&be_2), 4);
		tmp16 = htons(item.protocol());
		buffer.append(reinterpret_cast<const char *>(&tmp16), 2);

		buffer.append(reinterpret_cast<const char *>(&be_4), 4);
		tmp32 = htonl(item.source_ip());
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);

		buffer.append(reinterpret_cast<const char *>(&be_4), 4);
		tmp32 = htonl(item.destination_ip());
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);

		buffer.append(reinterpret_cast<const char *>(&be_2), 4);
		tmp16 = htons(item.source_port());
		buffer.append(reinterpret_cast<const char *>(&tmp16), 2);

		buffer.append(reinterpret_cast<const char *>(&be_2), 4);
		tmp16 = htons(item.destination_port());
		buffer.append(reinterpret_cast<const char *>(&tmp16), 2);

		tmp32 = htonl(static_cast<uint32_t>(item.type().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.type().data(), item.type().size());

		tmp32 = htonl(static_cast<uint32_t>(item.uri().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.uri().data(), item.uri().size());

		tmp32 = htonl(static_cast<uint32_t>(item.version().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.version().data(), item.version().size());

		tmp32 = htonl(static_cast<uint32_t>(item.host().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.host().data(), item.host().size());

		tmp32 = htonl(static_cast<uint32_t>(item.user_agent().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.user_agent().data(), item.user_agent().size());

		tmp32 = htonl(static_cast<uint32_t>(item.referer().size()));
		buffer.append(reinterpret_cast<const char *>(&tmp32), 4);
		buffer.append(item.referer().data(), item.referer().size());

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const HTTPRequest &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[6];

		values[0] = item.type().c_str();
		values[1] = item.uri().c_str();
		values[2] = item.version().c_str();
		values[3] = item.host().c_str();
		values[4] = item.user_agent().c_str();
		values[5] = item.referer().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.time().microseconds()) + "', "
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
	int16_t _binary_tuples;
};

template <>
class PostgreSQLTableFormat <HTTPResponse> {
  public:
	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "time_seconds"));
		_columns.push_back(std::make_pair("uint32", "time_microseconds"));
		_columns.push_back(std::make_pair("uint16", "protocol"));
		_columns.push_back(std::make_pair("uint32", "source_ip"));
		_columns.push_back(std::make_pair("uint32", "destination_ip"));
		_columns.push_back(std::make_pair("uint16", "source_port"));
		_columns.push_back(std::make_pair("uint16", "destination_port"));

		_columns.push_back(std::make_pair("TEXT", "version"));
		_columns.push_back(std::make_pair("TEXT", "status"));
		_columns.push_back(std::make_pair("TEXT", "reason"));
		_columns.push_back(std::make_pair("TEXT", "response"));
		_columns.push_back(std::make_pair("TEXT", "content_type"));
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

	bool copy_text(const HTTPResponse &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.time().seconds()) + '\t' +
					  lexical_cast<string>(item.time().microseconds()) + '\t' +
					  lexical_cast<string>((int) item.protocol()) + '\t' +
					  lexical_cast<string>(item.source_ip()) + '\t' +
					  lexical_cast<string>(item.destination_ip()) + '\t' +
					  lexical_cast<string>(item.source_port()) + '\t' +
					  lexical_cast<string>(item.destination_port()) + '\t');

		regex search("(\\\\)|([\\b])|(\\f)|(\\n)|(\\r)|(\\t)|(\\v)");
		string replace("(?1\\\\\\\\)(?2\\\\b)(?3\\\\f)"
					   "(?4\\\\n)(?5\\\\r)(?6\\\\t)(?7\\\\v)");

		string str(item.version());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.status());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.reason());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.response());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\t");
		str.assign(item.content_type());
		regex_replace(back_inserter(buffer),
					  str.begin(),
					  str.end(),
					  search,
					  replace,
					  match_default | format_all);
		buffer.append("\n");

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const HTTPResponse &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		const char * values[5];

		values[0] = item.version().c_str();
		values[1] = item.status().c_str();
		values[2] = item.reason().c_str();
		values[3] = item.response().c_str();
		values[4] = item.content_type().c_str();

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.time().seconds()) + "', "
				 "'" + lexical_cast<string>(
							item.time().microseconds()) + "', "
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
class PostgreSQLTableFormat <host_ip_index> {
  public:
	typedef host_ip_index value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		//_columns.push_back(std::make_pair("uint32 PRIMARY KEY", "host"));
		//_columns.push_back(std::make_pair("uint32 UNIQUE", "id"));
		_columns.push_back(std::make_pair("uint32", "host"));
		_columns.push_back(std::make_pair("uint32", "id"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.ip()) + '\t' +
					  lexical_cast<string>(item.index()) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.ip()) + "', "
				 "'" + lexical_cast<string>(
							item.index()) + "');");

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
class PostgreSQLTableFormat <host_index_pair> {
  public:
	typedef host_index_pair value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "internal_id"));
		_columns.push_back(std::make_pair("uint32", "external_id"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.internal_id()) + '\t' +
					  lexical_cast<string>(item.external_id()) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.internal_id()) + "', "
				 "'" + lexical_cast<string>(
							item.external_id()) + "');");

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
class PostgreSQLTableFormat <network_stats> {
  public:
	typedef network_stats value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("TEXT", "date"));
		_columns.push_back(std::make_pair("uint32", "internal_ip_count"));
		_columns.push_back(std::make_pair("uint32", "external_ip_count"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(item.date() + '\t' +
					  lexical_cast<string>(item.internal_ip_count()) + '\t' +
					  lexical_cast<string>(item.external_ip_count()) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ($1, "
				 "'" + lexical_cast<string>(
							item.internal_ip_count()) + "', "
				 "'" + lexical_cast<string>(
							item.external_ip_count()) + "');");

		const char * date_ptr(item.date().c_str());
		return PQexecParams(conn.connection(),
							s.c_str(),
							1,
							NULL,
							&date_ptr,
							NULL,
							NULL,
							0);
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <bandwidth_utilization_datum> {
  public:
	typedef bandwidth_utilization_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "interval_start"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.time_seconds) + '\t' +
					  lexical_cast<string>(item.ingress_bytes_per_second) + '\t' +
					  lexical_cast<string>(item.egress_bytes_per_second) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.time_seconds) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second) + "', "
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second) + "');");

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
class PostgreSQLTableFormat <host_bandwidth_utilization_datum> {
  public:
	typedef host_bandwidth_utilization_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "host"));
		_columns.push_back(std::make_pair("uint32", "interval_start"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second"));
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
		s.append(" PRIMARY KEY(host, interval_start));");

		return s;
	}

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.host) + '\t' +
					  lexical_cast<string>(item.time_seconds) + '\t' +
					  lexical_cast<string>(item.ingress_bytes_per_second) + '\t' +
					  lexical_cast<string>(item.egress_bytes_per_second) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.host) + "', "
				 "'" + lexical_cast<string>(
							item.time_seconds) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second) + "', "
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second) + "');");

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
class PostgreSQLTableFormat <bandwidth_content_datum> {
  public:
	typedef bandwidth_content_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "interval_start"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_plaintext"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_bmp_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_wav_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_compressed"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_jpeg_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_mp3_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_mpeg_video"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_encrypted"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_unknown"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_plaintext"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_bmp_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_wav_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_compressed"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_jpeg_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_mp3_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_mpeg_video"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_encrypted"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_unknown"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.time_seconds) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_plaintext) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_bmp_image) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_wav_audio) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_compressed) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_jpeg_image) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_mp3_audio) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_mpeg_video) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_encrypted) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_unknown) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_plaintext) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_bmp_image) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_wav_audio) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_compressed) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_jpeg_image) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_mp3_audio) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_mpeg_video) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_encrypted) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_unknown) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.time_seconds) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_plaintext) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_bmp_image) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_wav_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_compressed) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_jpeg_image) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_mp3_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_mpeg_video) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_encrypted) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_unknown) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_plaintext) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_bmp_image) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_wav_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_compressed) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_jpeg_image) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_mp3_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_mpeg_video) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_encrypted) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_unknown) + "');");

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
class PostgreSQLTableFormat <host_bandwidth_content_datum> {
  public:
	typedef host_bandwidth_content_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "host"));
		_columns.push_back(std::make_pair("uint32", "interval_start"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_plaintext"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_bmp_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_wav_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_compressed"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_jpeg_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_mp3_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_mpeg_video"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_encrypted"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "ingress_bytes_per_second_unknown"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_plaintext"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_bmp_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_wav_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_compressed"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_jpeg_image"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_mp3_audio"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_mpeg_video"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_encrypted"));
		_columns.push_back(std::make_pair("DOUBLE PRECISION", "egress_bytes_per_second_unknown"));
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
		s.append(" PRIMARY KEY(host, interval_start));");

		return s;
	}

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.host) + '\t' +
					  lexical_cast<string>(item.time_seconds) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_plaintext) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_bmp_image) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_wav_audio) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_compressed) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_jpeg_image) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_mp3_audio) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_mpeg_video) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_encrypted) + '\t' +
					  lexical_cast<string>(
							item.ingress_bytes_per_second_unknown) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_plaintext) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_bmp_image) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_wav_audio) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_compressed) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_jpeg_image) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_mp3_audio) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_mpeg_video) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_encrypted) + '\t' +
					  lexical_cast<string>(
							item.egress_bytes_per_second_unknown) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.host) + "', "
				 "'" + lexical_cast<string>(
							item.time_seconds) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_plaintext) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_bmp_image) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_wav_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_compressed) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_jpeg_image) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_mp3_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_mpeg_video) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_encrypted) + "', " +
				 "'" + lexical_cast<string>(
							item.ingress_bytes_per_second_unknown) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_plaintext) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_bmp_image) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_wav_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_compressed) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_jpeg_image) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_mp3_audio) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_mpeg_video) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_encrypted) + "', " +
				 "'" + lexical_cast<string>(
							item.egress_bytes_per_second_unknown) + "');");

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
class PostgreSQLTableFormat <as_exposure_datum> {
  public:
	typedef as_exposure_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint16", "asn"));
		_columns.push_back(std::make_pair("uint64", "internal_hosts_contacted"));
		_columns.push_back(std::make_pair("uint64", "ingress_bytes"));
		_columns.push_back(std::make_pair("uint64", "egress_bytes"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.asn) + '\t' +
					  lexical_cast<string>(item.internal_hosts_contacted) +
						'\t' +
					  lexical_cast<string>(item.ingress_bytes) + '\t' +
					  lexical_cast<string>(item.egress_bytes) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.asn) + "', "
				 "'" + lexical_cast<string>(
							item.internal_hosts_contacted) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes) + "', "
				 "'" + lexical_cast<string>(
							item.egress_bytes) + "');");

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
class PostgreSQLTableFormat <host_as_exposure_datum> {
  public:
	typedef host_as_exposure_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "host"));
		_columns.push_back(std::make_pair("uint16", "asn"));
		_columns.push_back(std::make_pair("uint64", "ingress_bytes"));
		_columns.push_back(std::make_pair("uint64", "egress_bytes"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.host) + '\t' +
					  lexical_cast<string>(item.asn) + '\t' +
					  lexical_cast<string>(item.ingress_bytes) + '\t' +
					  lexical_cast<string>(item.egress_bytes) + '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + lexical_cast<string>(
							item.host) + "', "
				 "'" + lexical_cast<string>(
							item.asn) + "', "
				 "'" + lexical_cast<string>(
							item.ingress_bytes) + "', "
				 "'" + lexical_cast<string>(
							item.egress_bytes) + "');");

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
class PostgreSQLTableFormat <browser_stat> {
  public:
	typedef browser_stat value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("TEXT", "browser"));
		_columns.push_back(std::make_pair("uint64", "internal_host_count"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(item.browser + '\t' +
					  lexical_cast<string>(item.internal_host_count) +
					  '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + item.browser + "', "
				 "'" + lexical_cast<string>(
							item.internal_host_count) + "');");

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
class PostgreSQLTableFormat <browser_version_stat> {
  public:
	typedef browser_version_stat value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("TEXT", "browser"));
		_columns.push_back(std::make_pair("TEXT", "version"));
		_columns.push_back(std::make_pair("uint64", "internal_host_count"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(item.browser + '\t' +
					  item.version + '\t' +
					  lexical_cast<string>(item.internal_host_count) +
					  '\n');

		return true;
	}

	PGresult * insert(PostgreSQLConnection &conn,
					  const std::string &schema,
					  const std::string &table,
					  const value_type &item)
	{
		if (!conn.isOpen()) {
			return false;
		}

		using namespace std;
		using namespace boost;

		string s("INSERT INTO \"" + schema + "\".\"" + table + "\" "
				 "VALUES ("
				 "'" + item.browser + "', "
				 "'" + item.version + "', "
				 "'" + lexical_cast<string>(
							item.internal_host_count) + "');");

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
class PostgreSQLTableFormat <dns_row> {
  public:
	typedef dns_row value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("bigserial", "id"));
		_columns.push_back(std::make_pair("uint32", "query_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "query_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "response_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "response_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "client_ip"));
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("TEXT", "query_name"));
		_columns.push_back(std::make_pair("uint16", "query_type"));
		_columns.push_back(std::make_pair("uint16", "response_code"));
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
		s.append("PRIMARY KEY (id) );");

		return s;
	}

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		std::vector<char> escaped_name(item.query_name.size() * 2 + 1);
		size_t name_len(PQescapeString(&escaped_name.front(),
									   item.query_name.data(),
									   item.query_name.size()));

		buffer.append(lexical_cast<string>(item.id) + '\t' +
					  lexical_cast<string>(item.query_time.seconds()) + '\t' +
					  lexical_cast<string>(item.query_time.microseconds()) +
						'\t' +
					  lexical_cast<string>(item.response_time.seconds()) +
						'\t' +
					  lexical_cast<string>(item.response_time.microseconds()) +
						'\t' +
					  lexical_cast<string>(item.client_ip) + '\t' +
					  lexical_cast<string>(item.server_ip) + '\t');
		buffer.append(&escaped_name.front(), name_len);
		buffer.append('\t' + lexical_cast<string>(item.query_type) + '\t' +
					  lexical_cast<string>(item.response_code) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <dns_response_row> {
  public:
	typedef dns_response_row value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("int8", "dns_id"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		std::vector<char> escaped_name(item.name.size() * 2 + 1);
		size_t name_len(PQescapeString(&escaped_name.front(),
									   item.name.data(),
									   item.name.size()));

		size_t rd_len(0);
		const char *rd(
			reinterpret_cast <const char *>(
				PQescapeBytea(reinterpret_cast<const unsigned char *>(
								item.resource_data.data()),
							  item.resource_data.length(),
							  &rd_len)));

		buffer.append(lexical_cast<string>(item.dns_id) + '\t');
		buffer.append(&escaped_name.front(), name_len);
		buffer.append('\t' + lexical_cast<string>(item.type) + '\t');
		buffer.append(rd, rd_len);
		buffer.append('\t' +
					  lexical_cast<string>(item.ttl) + '\n');
		PQfreemem(const_cast<void *>(reinterpret_cast<const void *>(rd)));
		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <top_url_datum> {
  public:
	typedef top_url_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("text", "url"));
		_columns.push_back(std::make_pair("bigint", "request_count"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_microseconds"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.server_ip) + '\t');

		regex search("(\\\\)|([\\b])|(\\f)|(\\n)|(\\r)|(\\t)|(\\v)");
		string replace("(?1\\\\\\\\)(?2\\\\b)(?3\\\\f)"
					   "(?4\\\\n)(?5\\\\r)(?6\\\\t)(?7\\\\v)");

		regex_replace(back_inserter(buffer),
					  item.url.begin(),
					  item.url.end(),
					  search,
					  replace,
					  match_default | format_all);

		buffer.append("\t");

		buffer.append(lexical_cast<string>(item.request_count) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.microseconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.microseconds()) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <top_host_datum> {
  public:
	typedef top_host_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("text", "host"));
		_columns.push_back(std::make_pair("bigint", "request_count"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_microseconds"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.server_ip) + '\t' +
					  item.host + '\t' +
					  lexical_cast<string>(item.request_count) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.microseconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.microseconds()) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <web_server_browser_datum> {
  public:
	typedef web_server_browser_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("text", "browser"));
		_columns.push_back(std::make_pair("text", "browser_version"));
		_columns.push_back(std::make_pair("bigint", "request_count"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_microseconds"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.server_ip) + '\t' +
					  item.browser.first + '\t' +
					  item.browser.second + '\t' +
					  lexical_cast<string>(item.request_count) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.microseconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.microseconds()) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <web_server_crawler_datum> {
  public:
	typedef web_server_crawler_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("uint32", "client_ip"));
		_columns.push_back(std::make_pair("text", "user_agent"));
		_columns.push_back(std::make_pair("bigint", "request_count"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "first_request_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "last_request_time_microseconds"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.server_ip) + '\t' +
					  lexical_cast<string>(item.crawler.first) + '\t' +
					  item.crawler.second + '\t' +
					  lexical_cast<string>(item.request_count) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.first_request_time.microseconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.last_request_time.microseconds()) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

template <>
class PostgreSQLTableFormat <web_server_server_datum> {
  public:
	typedef web_server_server_datum value_type;

	PostgreSQLTableFormat()
		:_columns()
	{
		_columns.push_back(std::make_pair("uint32", "server_ip"));
		_columns.push_back(std::make_pair("text", "server"));
		_columns.push_back(std::make_pair("bigint", "response_count"));
		_columns.push_back(std::make_pair("uint32", "first_response_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "first_response_time_microseconds"));
		_columns.push_back(std::make_pair("uint32", "last_response_time_seconds"));
		_columns.push_back(std::make_pair("uint32", "last_response_time_microseconds"));
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

	bool copy_text(const value_type &item,
				   std::string &buffer)
	{
		using namespace std;
		using namespace boost;

		buffer.append(lexical_cast<string>(item.server_ip) + '\t' +
					  item.server + '\t' +
					  lexical_cast<string>(item.response_count) + '\t' +
					  lexical_cast<string>(
						item.first_response_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.first_response_time.microseconds()) + '\t' +
					  lexical_cast<string>(
						item.last_response_time.seconds()) + '\t' +
					  lexical_cast<string>(
						item.last_response_time.microseconds()) + '\n');

		return true;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

#endif
