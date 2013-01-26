#ifndef HBFSYNAPPARGUMENTS_H
#define HBFSYNAPPARGUMENTS_H

#include <string>
#include <boost/filesystem.hpp>

#include "SynappArguments.h"

namespace vn {
namespace arl {
namespace shared {

/// \brief The HBFSynappArguments class.
///
/// This class is used by a Synapp to obtain the command-line parameters.
class HBFSynappArguments : public SynappArguments {
  public:
	/// \brief Constructor
	/// \param argc The number of arguments, including the executable name
	/// \param argv The arguments, including the executable name
	///
	/// Construct a HBFSynappArguments.
	explicit HBFSynappArguments(int argc, char **argv);

	/// \brief Get the input directory
	/// \returns the value of the input-dir command-line argument
	const boost::filesystem::path & inputDir() const;

	/// \brief Get the output directory
	/// \returns the value of the output-dir command-line argument
	///
	/// Note: If an output-file command-line argument is set, the path
	/// returned by this function is empty.
	const boost::filesystem::path & outputDir() const;
	
	/// \brief Get the config file
	/// \returns the value of the config-file command-line argument, if set,
	/// or an empty path otherwise.
	const boost::filesystem::path & configFile() const;

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

	/// The value of the input-dir command-line option
	boost::filesystem::path _inputDir;

	/// The value of the output-dir command-line option
	boost::filesystem::path _outputDir;
	
	/// The value of the config-file command-line option
	boost::filesystem::path _configFile;

	/// Whether or not the help command-line option was specified
	bool _help;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
