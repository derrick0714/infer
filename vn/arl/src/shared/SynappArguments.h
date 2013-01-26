#ifndef SYNAPPARGUMENTS_H
#define SYNAPPARGUMENTS_H

#include <ostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace vn {
namespace arl {
namespace shared {

class SynappArguments;

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out,
						  const vn::arl::shared::SynappArguments &args);

namespace vn {
namespace arl {
namespace shared {

/// \brief The SynappArguments class.
///
/// This class is used by a Synapp to obtain the command-line parameters.
class SynappArguments {
  public:
	/// \brief Constructor
	/// \param argc The number of arguments, including the executable name
	/// \param argv The arguments, including the executable name
	///
	/// Construct a SynappArguments.
	explicit SynappArguments(int argc, char **argv);

	SynappArguments(const SynappArguments &sa);

	/// \brief Virtual destructor
	virtual ~SynappArguments();

	/// \brief Get the error string
	/// \returns the error string.
	const std::string & error() const;

	/// \brief Boolean cast operator
	/// \returns true if there is no error
	///
	/// This is the means by which clients will test for an error.
	operator bool() const;

	/// \brief Write the command-line options to an std::ostream
	/// \param out The std::ostream to write to
	/// \param args The SynappArguments whose options are to be written
	///
	/// This is used to write the available options to an std::ostream. The
	/// intended usage is for this to be used by a client to display the 
	/// options available in the event of an error.
	friend std::ostream& ::operator<< (std::ostream &out, const SynappArguments &args);

  protected:
	/// \brief Set the options' descriptions
	///
	/// This function populates the boost::program_options::options_description
	virtual void setOptionsDescriptions() = 0;

	/// \brief Parse the command line options
	///
	/// This function parses the arguments on the command line and stores their
	/// values accordingly. Sets _error is an error occurs.
	virtual void parseOptions() = 0;

	int argc;

	char **argv;

	/// \brief Read the options from the command line
	/// \param vals Where to put the command-line argument values
	///
	/// This function is to be called from parseOptions() in a derived class.
	/// See parseOptions() for details.
	bool _parseOptions(boost::program_options::variables_map &vals);

	/// The error status
	bool _error;

	/// The error message
	std::string _errorMsg;

	/// The description of valid command-line options
	boost::program_options::options_description arguments;

  private:
	SynappArguments & operator=(const SynappArguments &rhs);
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
