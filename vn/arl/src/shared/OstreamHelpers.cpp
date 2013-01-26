#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include "OstreamHelpers.h"
#include "Base64.h"
#include "HBFResult.hpp"
#include "HTTP.h"
//#include "HTTPResult.hpp"
#include "HBFHTTPResult.hpp"

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HBFResult &result)
{
	using namespace boost::posix_time;
	using namespace vn::arl::shared;

	os	<< boost::asio::ip::address_v4(result.sourceIP()) << ' '
		<< boost::asio::ip::address_v4(result.destinationIP()) << ' '
		<< result.sourcePort() << ' '
		<< result.destinationPort() << ' '
		<< to_simple_string((ptime) result.startTime()) << ' '
		<< to_simple_string((ptime) result.endTime()) << ' '
		<< Base64::encode(result.match().data(), result.match().length());

	return os;
}

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HTTP &http)
{
	using namespace boost::posix_time;
	using namespace vn::arl::shared;

	os	<< boost::asio::ip::address_v4(http.sourceIP()) << ' '
		<< boost::asio::ip::address_v4(http.destinationIP()) << ' '
		<< http.sourcePort() << ' '
		<< http.destinationPort() << ' '
		<< to_simple_string((ptime) http.time());

	switch (http.type()) {
	  case 'q':
		os	<< ' ' << http.type() << ' '
			<< '"' << http.requestType() << "\" "
			<< '"' << http.uri() << "\" "
			<< '"' << http.version() << "\" "
			<< '"' << http.host() << "\" "
			<< '"' << http.userAgent() << "\" "
			<< '"' << http.referer() << '"';
		break;

	  case 's':
		os	<< ' ' << http.type() << ' '
			<< '"' << http.version() << "\" "
			<< '"' << http.status() << "\" "
			<< '"' << http.reason() << "\" "
			<< '"' << http.response() << "\" "
			<< '"' << http.contentType() << '"';
		break;

	  default:
		break;
	}

	return os;
}

/*
std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HTTPResult &result);
*/

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HBFHTTPResult &result)
{
	using namespace vn::arl::shared;

	const HTTPResult &httpResult(result.http());
	const HTTP &request(httpResult.request());
	const HTTP &response(httpResult.response());

	os	<< result.hbf() << ' '
		<< '"' << request.requestType() << "\" "
		<< '"' << request.uri() << "\" "
		<< '"' << request.version() << "\" "
		<< '"' << request.host() << "\" "
		<< '"' << request.userAgent() << "\" "
		<< '"' << request.referer() << "\" "
		<< '"' << response.version() << "\" "
		<< '"' << response.status() << "\" "
		<< '"' << response.reason() << "\" "
		<< '"' << response.response() << "\" "
		<< '"' << response.contentType() << '"';

	return os;
}
