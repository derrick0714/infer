#ifndef PAYLOADSEARCHMANAGERARGUMENTS_H
#define PAYLOADSEARCHMANAGERARGUMENTS_H

#include <string>
#include <boost/filesystem.hpp>

#include "SynappArguments.h"
#include "TimeStamp.h"
#include "IPv4FlowMatcher.hpp"
#include "HBF.h"

namespace vn {
namespace arl {
namespace shared {

/// \brief The HBFSynappArguments class.
///
/// This class is used by a Synapp to obtain the command-line parameters.
class PayloadSearchManagerArguments : public SynappArguments {
  public:
	/// \brief Constructor
	/// \param argc The number of arguments, including the executable name
	/// \param argv The arguments, including the executable name
	///
	/// Construct a HBFSynappArguments.
	explicit PayloadSearchManagerArguments(int argc, char **argv);

	/// \brief Get the start time
	/// \returns the value of the start-time command-line argument
	const TimeStamp & startTime() const;

	/// \brief Get the end time
	/// \returns the value of the end-time command-line argument
	const TimeStamp & endTime() const;

	/// \brief Get the input directory
	/// \returns the value of the input-dir command-line argument
	const boost::filesystem::path & inputDir() const;

	/// \brief Get the input file
	/// \returns the value of the input-file command-line argument
	const boost::filesystem::path & inputFile() const;

	/// \brief Get the input data
	/// \returns the decoded value of the input-data command-line argument
	const std::string & inputData() const;

	/// \brief Get the query ID
	/// \returns the query ID
	const std::string & queryID() const;

	/// \brief Get the query length
	/// \returns the value of the query-length command-line argument
	size_t queryLength() const;

	/// \brief Get the match length
	/// \returns the value of the match-length command-line argument
	size_t matchLength() const;
	/// \brief Get the output directory
	/// \returns the value of the output-dir command-line argument
	///
	/// Note: If an output-file command-line argument is set, the path
	/// returned by this function is empty.
	//const boost::filesystem::path & outputDir() const;
	
	/// \brief Get the config file
	/// \returns the value of the config-file command-line argument, if set,
	/// or an empty path otherwise.
	const boost::filesystem::path & configFile() const;

	IPv4FlowMatcher ipv4FlowMatcher() const;

	/// \brief Get the help argument
	/// \returns true if the help command-line argument was specified.
	bool help() const;

  private:
	/// \brief Set the options' descriptions
	///
	/// This function populates the boost::program_options::options_description
	void setOptionsDescriptions();

	/// \brief Parse the command line options
	///
	/// This function parses the arguments on the command line and stores their
	/// values accordingly. Sets _error is an error occurs.
	void parseOptions();

	bool parseNetwork(const std::string &str, IPv4Network &net);

	bool parsePortRange(const std::string &str,
						std::pair <uint16_t, uint16_t> &ports);

	bool parseTime(TimeStamp &t, const std::string &str);

	/// The value of the start-time command-line option
	TimeStamp _startTime;

	/// The value of the end-time command-line option
	TimeStamp _endTime;

	/// The value of the input-dir command-line option
	boost::filesystem::path _inputDir;

	/// The value of the query-id command-line option
	std::string _queryID;

	/// The value of the input-file command-line option
	boost::filesystem::path _inputFile;

	/// The decoded value of the input-data command-line option
	std::string _inputData;

	/// The value of the query-length command-line option
	size_t _queryLength;

	/// The value of the match-length command-line option
	size_t _matchLength;
	
	/// The value of the output-dir command-line option
	//boost::filesystem::path _outputDir;
	
	/// The value of the config-file command-line option
	boost::filesystem::path _configFile;

	/// The IPv4FlowMatcher generated from the optional command-line options
	IPv4FlowMatcher _ipv4FlowMatcher;

	/// Whether or not the help command-line option was specified
	bool _help;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
