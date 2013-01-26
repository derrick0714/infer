#include <time.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <fstream>

#include "PayloadSearchManagerArguments.h"
#include "Base64.h"

PayloadSearchManagerArguments::PayloadSearchManagerArguments(int argc, char **argv)
	:SynappArguments(argc, argv),
	 _startTime(),
	 _endTime(),
	 _inputDir(),
	 _queryID(),
	 _inputFile(),
	 _inputData(),
	 _queryLength(0),
	 _matchLength(0),
//	 _outputDir(),
	 _configFile("/usr/local/etc/infer.conf"),
	 _ipv4FlowMatcher(),
	 _help(false)
{
	setOptionsDescriptions();
	parseOptions();
}

const TimeStamp & PayloadSearchManagerArguments::startTime() const {
	return _startTime;
}

const TimeStamp & PayloadSearchManagerArguments::endTime() const {
	return _endTime;
}

const boost::filesystem::path & PayloadSearchManagerArguments::inputDir() const {
	return _inputDir;
}

const std::string & PayloadSearchManagerArguments::queryID() const {
	return _queryID;
}

const boost::filesystem::path & PayloadSearchManagerArguments::inputFile() const {
	return _inputFile;
}

const std::string & PayloadSearchManagerArguments::inputData() const {
	return _inputData;
}

size_t PayloadSearchManagerArguments::queryLength() const {
	return _queryLength;
}

size_t PayloadSearchManagerArguments::matchLength() const {
	return _matchLength;
}

/*
const boost::filesystem::path & PayloadSearchManagerArguments::outputDir() const {
	return _outputDir;
}
*/

const boost::filesystem::path & PayloadSearchManagerArguments::configFile() const {
	return _configFile;
}


IPv4FlowMatcher PayloadSearchManagerArguments::ipv4FlowMatcher() const {
	return _ipv4FlowMatcher;
}

bool PayloadSearchManagerArguments::help() const {
	return _help;
}

void PayloadSearchManagerArguments::setOptionsDescriptions() {
	boost::program_options::options_description requiredArgs
												("Synapp Required Arguments");
	requiredArgs.add_options()
		(
			"start-time,s",
			boost::program_options::value <std::string>(),
			"start time of data to process"
		)
		(
			"end-time,e",
			boost::program_options::value <std::string>(),
			"end time of data to process"
		)
		(
			"input-dir,i",
			boost::program_options::value <std::string>(),
			"directory from which to read data files"
		)
		(
			"query-id,t",
			boost::program_options::value <std::string>(),
			"identification string for the query being issued"
		)
		/*
		(
			"output-dir,o",
			boost::program_options::value <std::string>(),
			"directory in which to write synopses"
		)
		*/
		(
			"input-file,f",
			boost::program_options::value <std::string>(),
			"the file containing data to be queried for"
		)
		(
			"query-length,q",
			boost::program_options::value <size_t>(),
			"the number of bytes to use for the query"
		)
		(
			"match-length,m",
			boost::program_options::value <size_t>(),
			"the minimum number of bytes to consider a match"
		)
	;

	boost::program_options::options_description optionalArgs
												("Synapp Optional Arguments");
	optionalArgs.add_options()
		(
			"config-file,c",
			boost::program_options::value <std::string>(),
			"file from which to read additional configuration options."
		)
		(
			"input-data,d",
			boost::program_options::value <std::string>(),
			"base64-encoded data to query for"
		)
		(
			"network,n",
			boost::program_options::value <std::vector <std::string> >(),
			"a network that either the source or destination must match"
		)
		(
			"source-net",
			boost::program_options::value <std::vector <std::string> >(),
			"a network that the source must match. Cannot be combined with "
			"the network option"
		)
		(
			"dest-net",
			boost::program_options::value <std::vector <std::string> >(),
			"a network that the destination must match. Cannot be combined "
			"with the network option"
		)
		(
			"port,p",
			boost::program_options::value <std::vector <std::string> >(),
			"a port number or range that either the source or destination "
			"must match"
		)
		(
			"source-port",
			boost::program_options::value <std::vector <std::string> >(),
			"a port number or range that the source must match. Cannot be "
			"combined with the port option"
		)
		(
			"dest-port",
			boost::program_options::value <std::vector <std::string> >(),
			"a port number or range that the destination must match. Cannot "
			"be combined with the port option"
		)
		(
			"help,h",
			"prints a help message to stdout"
		)
	;

	arguments.add(requiredArgs);
	arguments.add(optionalArgs);
}

void PayloadSearchManagerArguments::parseOptions() {
	boost::program_options::variables_map opts;
	if (!_parseOptions(opts)) {
		return;
	}

	// optional options
	if (opts.count("help")) {
		_help = true;
		return;
	}

	if (opts.count("config-file")) {
		_configFile = boost::filesystem::path
								(opts["config-file"].as<std::string>());
	}
	if (!boost::filesystem::exists(_configFile)) {
		_errorMsg.assign("invalid config-file: file \"");
		_errorMsg.append(_configFile.string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_regular_file(_configFile)) {
		_errorMsg.assign("invalid config-file: file \"");
		_errorMsg.append(_configFile.string());
		_errorMsg.append("\" is not a regular file.");
		_error = true;
		return;
	}	

	if (opts.count("network") &&
		(opts.count("source-net") || opts.count("dest-net")))
	{
		_error = true;
		_errorMsg.assign("the network option cannot be combined with either of"
						 " the source-net or dest-net options");
		return;
	}
	if (opts.count("port") &&
		(opts.count("source-port") || opts.count("dest-port")))
	{
		_error = true;
		_errorMsg.assign("the port option cannot be combined with either of "
						 "the source-port or dest-port options");
		return;
	}

	if (opts.count("network")) {
		const std::vector <std::string> & nets =
			opts["network"].as<std::vector <std::string> >();

		IPv4Network ipv4Network;

		for (std::vector <std::string>::const_iterator
				it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			// parse CIDR network
			if (!parseNetwork(*it, ipv4Network)) {
				return;
			}

			_ipv4FlowMatcher.addEitherNetwork(ipv4Network);
		}
	}

	if (opts.count("source-net")) {
		const std::vector <std::string> & nets =
			opts["source-net"].as<std::vector <std::string> >();

		IPv4Network ipv4Network;

		for (std::vector <std::string>::const_iterator
				it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			// parse CIDR network
			if (!parseNetwork(*it, ipv4Network)) {
				return;
			}

			_ipv4FlowMatcher.addSourceNetwork(ipv4Network);
		}
	}

	if (opts.count("dest-net")) {
		const std::vector <std::string> & nets =
			opts["dest-net"].as<std::vector <std::string> >();

		IPv4Network ipv4Network;

		for (std::vector <std::string>::const_iterator
				it(nets.begin());
			 it != nets.end();
			 ++it)
		{
			// parse CIDR network
			if (!parseNetwork(*it, ipv4Network)) {
				return;
			}

			_ipv4FlowMatcher.addDestinationNetwork(ipv4Network);
		}
	}

	if (opts.count("port")) {
		const std::vector <std::string> & ports =
			opts["port"].as<std::vector <std::string> >();
		
		std::pair <uint16_t, uint16_t> portPair;

		for (std::vector <std::string>::const_iterator
				it(ports.begin());
			 it != ports.end();
			 ++it)
		{
			// parse port pair
			if (!parsePortRange(*it, portPair))
			{
				return;
			}

			_ipv4FlowMatcher.addEitherPortRange(portPair);
		}
	}

	if (opts.count("source-port")) {
		const std::vector <std::string> & ports =
			opts["source-port"].as<std::vector <std::string> >();
		
		std::pair <uint16_t, uint16_t> portPair;

		for (std::vector <std::string>::const_iterator
				it(ports.begin());
			 it != ports.end();
			 ++it)
		{
			// parse port pair
			if (!parsePortRange(*it, portPair))	{
				return;
			}

			_ipv4FlowMatcher.addSourcePortRange(portPair);
		}
	}

	if (opts.count("dest-port")) {
		const std::vector <std::string> & ports =
			opts["dest-port"].as<std::vector <std::string> >();
		
		std::pair <uint16_t, uint16_t> portPair;

		for (std::vector <std::string>::const_iterator
				it(ports.begin());
			 it != ports.end();
			 ++it)
		{
			// parse port pair
			if (!parsePortRange(*it, portPair))	{
				return;
			}

			_ipv4FlowMatcher.addDestinationPortRange(portPair);
		}
	}
	if (opts.count("input-data")) {
		if (opts.count("input-file")) {
			_errorMsg.assign("input-data and input-file cannot both be "
							 "specified");
			_error = true;
			return;
		}
		_inputData = opts["input-data"].as<std::string>();
		_inputData = Base64::decode(_inputData);
	}

	// required opts
	if (!opts.count("start-time")) {
		_errorMsg.assign("start-time not specified.");
		_error = true;
		return;
	}
	if (!parseTime(_startTime, opts["start-time"].as<std::string>().c_str())) {
		_error = true;
		_errorMsg.assign("invalid start-time: format is: "
						 "%Y[-%m[-%d[ %H[:%M[:%S]]]]]");
		return;
	}

	if (!opts.count("end-time")) {
		_errorMsg.assign("end-time not specified.");
		_error = true;
		return;
	}
	if (!parseTime(_endTime, opts["end-time"].as<std::string>().c_str())) {
		_error = true;
		_errorMsg.assign("invalid end-time: format is: "
						 "%Y[-%m[-%d[ %H[:%M[:%S]]]]]");
		return;
	}

	if (_startTime >= _endTime) {
		_errorMsg.assign("invalid time range: start-time >= end-time.");
		_error = true;
		return;
	}

	if (!opts.count("input-dir")) {
		_errorMsg.assign("input-dir not specified.");
		_error = true;
		return;
	}
	_inputDir = boost::filesystem::path(opts["input-dir"].as<std::string>());
	if (!boost::filesystem::exists(_inputDir)) {
		_errorMsg.assign("invalid input-dir: directory \"");
		_errorMsg.append(_inputDir.string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_directory(_inputDir)) {
		_errorMsg.assign("invalid input-dir: \"");
		_errorMsg.append(_inputDir.string());
		_errorMsg.append("\" is not a directory.");
		_error = true;
		return;
	}
	if (!opts.count("query-id")) {
		_errorMsg.assign("query-id not specified.");
		_error = true;
		return;
	}
	_queryID = opts["query-id"].as<std::string>();
	if (_queryID.empty()) {
		_errorMsg.assign("query-id cannot be empty.");
		_error = true;
		return;
	}
	if (!opts.count("query-length")) {
		_error = true;
		_errorMsg.assign("query-length not specified.");
		return;
	}
	_queryLength = opts["query-length"].as<size_t>();
	if (!opts.count("input-file") && !opts.count("input-data")) {
		_errorMsg.assign("input-file or input-data must be specified.");
		_error = true;
		return;
	}
	if (opts.count("input-file")) {
		_inputFile = boost::filesystem::path(opts["input-file"].as<std::string>());
		if (!boost::filesystem::exists(_inputFile)) {
			_errorMsg.assign("invalid input-file: file \"");
			_errorMsg.append(_inputFile.string());
			_errorMsg.append("\" does not exist.");
			_error = true;
			return;
		}
		if (!boost::filesystem::is_regular_file(_inputFile)) {
			_errorMsg.assign("invalid input-file: file \"");
			_errorMsg.append(_inputFile.string());
			_errorMsg.append("\" is not a regular file.");
			_error = true;
			return;
		}
		// open the file used for the query
		std::ifstream fin;
		fin.open(_inputFile.string().c_str());
		if (!fin.good()) {
			_errorMsg.assign("unable to open input-file");
			_error = true;
			return;
		}

		// fill the query string
		_inputData.resize(_queryLength);
		fin.read(const_cast<char *>(_inputData.data()), _queryLength);
		if (!fin.good()) {
			_errorMsg.assign("unable to read from input file");
			_error = true;
			return;
		}
		_inputData.resize(fin.gcount());

		// close the input file
		fin.close();
		if (!fin.good()) {
			_errorMsg.assign("unable to close input file");
			_error = true;
			return;
		}
	}
	if (_queryLength > _inputData.length()) {
		_error = true;
		_errorMsg.assign("invalid query-length: query-length must be less "
						 "than or equal to the amount of input data.");
		return;
	}
	if (_queryLength % HBF::BlockSize != 0) {
		_error = true;
		_errorMsg.assign("invalid query-length: query-length must be a "
						 "multiple of ");
		_errorMsg.append(boost::lexical_cast<std::string>
									(static_cast<size_t>(HBF::BlockSize)));
		return;
	}
	if (!opts.count("match-length")) {
		_error = true;
		_errorMsg.assign("match-length not specified.");
		return;
	}
	_matchLength = opts["match-length"].as<size_t>();
	if (_matchLength > _queryLength) {
		_error = true;
		_errorMsg.assign("invalid match-length: match length must be less "
						 "than or equal to query-length");
		return;
	}
	if (_matchLength % HBF::BlockSize != 0) {
		_error = true;
		_errorMsg.assign("invalid match-length: match-length must be a "
						 "multiple of ");
		_errorMsg.append(boost::lexical_cast<std::string>
									(static_cast<size_t>(HBF::BlockSize)));
		return;
	}
/*
	if (!opts.count("output-dir")) {
		_errorMsg.assign("output-dir not specified.");
		_error = true;
		return;
	}
	_outputDir = boost::filesystem::path(opts["output-dir"].as<std::string>());
	if (!boost::filesystem::exists(_outputDir)) {
		_errorMsg.assign("invalid output-dir: directory \"");
		_errorMsg.append(_outputDir.string());
		_errorMsg.append("\" does not exist.");
		_error = true;
		return;
	}
	if (!boost::filesystem::is_directory(_outputDir)) {
		_errorMsg.assign("invalid output-dir: \"");
		_errorMsg.append(_outputDir.string());
		_errorMsg.append("\" is not a directory.");
		_error = true;
		return;
	}
*/
}

bool PayloadSearchManagerArguments::parseNetwork(const std::string &str,
										   IPv4Network &net)
{
	using namespace std;
	using namespace boost::asio::ip;
	using namespace boost::system;

	string::size_type pos(str.find('/'));

	address_v4 ip;
	try {
		ip = address_v4::from_string(str.substr(0,pos));
	} catch (const system_error &e) {
		_error = true;
		_errorMsg.assign("invalid CIDR block: '");
		_errorMsg.append(str + "': bad IP address");
		return false;
	}

	uint16_t mask;
	if (pos != string::npos) {
		// get mask
		try {
			mask = boost::lexical_cast<uint32_t>(str.substr(pos+1));
		} catch (const boost::bad_lexical_cast &e) {
			_error = true;
			_errorMsg.assign("invalid CIDR block: '");
			_errorMsg.append(str + "': bad netmask");
			return false;
		}
	} else {
		mask = 32;
	}
	if (mask == 0 || mask > 32) {
		_error = true;
		_errorMsg.assign("invalid CIDR block: '");
		_errorMsg.append(str + "': netmask out of range");
		return false;
	}

	if (!net.set(ip.to_ulong(), 0xffffffff << (32 - mask))) {
		_error = true;
		_errorMsg.assign("invalid CIDR block: '");
		_errorMsg.append(str + "': network/netmask mismatch");
		return false;
	}

	return true;
}

bool PayloadSearchManagerArguments::parsePortRange(const std::string &str,
											 std::pair <uint16_t, uint16_t> 
											 					&ports)
{
	std::string::size_type pos(str.find('-'));
	try {
		if (pos != std::string::npos) {
			// we have a range
			if (pos != 0) {
				ports.first = boost::lexical_cast<uint16_t>(str.substr(0,pos));
			} else {
				ports.first = std::numeric_limits<uint16_t>::min();
			}

			if (pos != str.size() - 1) {
				ports.second = boost::lexical_cast<uint16_t>(str.substr(pos+1));
			} else {
				ports.second = std::numeric_limits<uint16_t>::max();
			}
		} else {
			// no range
			ports.first = ports.second = boost::lexical_cast<uint16_t>(str);
		}
	} catch (const boost::bad_lexical_cast) {
		_error = true;
		_errorMsg.assign("invalid port or port range: '");
		_errorMsg.append(str + '\'');
		return false;
	}

	if (ports.first > ports.second) {
		_error = true;
		_errorMsg.assign("invalid port or port range: '");
		_errorMsg.append(str + '\'');
		return false;
	}

	return true;
}

bool PayloadSearchManagerArguments::parseTime(TimeStamp &t, const std::string &str)
{
	struct tm tmpTime;
	std::string format("%Y-%m-%d %H:%M:%S");

	time_t tmpTimet(0);
	gmtime_r(&tmpTimet, &tmpTime);

	char *ret;
	while (format.size() > 2 &&
		  ((ret = strptime(str.c_str(), format.c_str(), &tmpTime)) == NULL ||
		   (*ret != '\0')))
	{
		format.resize(format.size() - 3);
	}
	if (ret == NULL || *ret != '\0') {
		if ((ret = strptime(str.c_str(), format.c_str(), &tmpTime)) == NULL ||
			(*ret != '\0'))
		{
			return false;
		}
	}

	tmpTimet = timegm(&tmpTime);

	t.set(static_cast <uint32_t>(tmpTimet), 0);
	return true;
}
