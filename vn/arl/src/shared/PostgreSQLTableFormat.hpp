#ifndef POSTGRESQLTABLEFORMAT_HPP
#define POSTGRESQLTABLEFORMAT_HPP

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <libpq-fe.h>

#include "HBFHTTPResult.hpp"
#include "Base64.h"
#include "PostgreSQLConnection.hpp"

namespace vn {
namespace arl {
namespace shared {

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

		if (result.http().request().type() == 0) {
			values[0] = NULL; // httpRequestVersion
			values[1] = NULL; // httpRequestType
			values[2] = NULL; // httpURI
			values[3] = NULL; // httpHost
			values[4] = NULL; // httpUserAgent
			values[5] = NULL; // httpReferer
		} else {
			values[0] = result.http().request().version().c_str();
			values[1] = result.http().request().requestType().c_str();
			values[2] = result.http().request().uri().c_str();
			values[3] = result.http().request().host().c_str();
			values[4] = result.http().request().userAgent().c_str();
			values[5] = result.http().request().referer().c_str();
		}
		if (result.http().response().type() == 0) {
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
			values[10] = result.http().response().contentType().c_str();
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

	std::string makeInsertString(const std::string &schema,
								 const std::string &table,
								 const HBFHTTPResult &result)
	{
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
									  result.hbf().match().length()) + "', ");

		if (result.http().request().type() == 0) {
			s.append("NULL, "   // httpRequestVersion
					 "NULL, "   // httpRequestType
					 "NULL, "   // httpURI
					 "NULL, "   // httpHost
					 "NULL, "   // httpUserAgent
					 "NULL, "); // httpReferer
		} else {
			s.append("'" + result.http().request().version() + "', "
					 "'" + result.http().request().requestType() + "', "
					 "'" + result.http().request().uri() + "', "
					 "'" + result.http().request().host() + "', "
					 "'" + result.http().request().userAgent() + "', "
					 "'" + result.http().request().referer() + "', ");
		}
		if (result.http().response().type() == 0) {
			s.append("NULL, " // httpResponseVersion
					 "NULL, " // httpStatus
					 "NULL, " // httpResponse
					 "NULL, " // httpReason
					 "NULL"); // httpContentType
		} else {
			s.append("'" + result.http().response().version() + "', "
					 "'" + result.http().response().status() + "', "
					 "'" + result.http().response().response() + "', "
					 "'" + result.http().response().reason() + "', "
					 "'" + result.http().response().contentType() + "'");
		}

		s.append(");");

		return s;
	}

  private:
	std::vector <std::pair <std::string, std::string> > _columns;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
