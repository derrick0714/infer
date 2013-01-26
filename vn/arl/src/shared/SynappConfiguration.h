#ifndef SYNAPPCONFIGURATION_H
#define SYNAPPCONFIGURATION_H

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace vn {
namespace arl {
namespace shared {

class SynappConfiguration;

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out,
						  const vn::arl::shared::SynappConfiguration &conf);

namespace vn {
namespace arl {
namespace shared {

/// \brief The SynappConfiguration abstract base class.
///
/// This is the abstract base class for a SynappConfiguration object. Any
/// Synapp that requires configuration parameters that are to be read from a
/// configuration file shall use an object that derives from this class to
/// access the configuration parameters.
class SynappConfiguration {
  public:
	/// \brief Constructor
	/// \param fileName The location of the configuration file to be read
	explicit SynappConfiguration(const boost::filesystem::path &fileName);

	/// \brief Virtual destructor
	virtual ~SynappConfiguration();

	/// \brief Get the error string
	/// \returns the error string.
	const std::string &error() const;

	/// \brief Boolean cast operator
	/// \returns true if there is no error
	///
	/// This is the means by which clients will test for an error.
	operator bool() const;

	/// \brief Write the configuration options to an std::ostream
	/// \param out The std::ostream to write to
	/// \param conf The SynappConfiguration whose options are to be written
	///
	/// This is used to write the available options to an std::ostream. The
	/// intended usage is for this to be used by a client to display the 
	/// options available in the event of an error.
	friend std::ostream& ::operator<< (std::ostream &out, const SynappConfiguration &conf);

  protected:
	/// \brief Set options descriptions
	/// 
	/// This function is to be implemented by any derived class. It is the
	/// means by which derived classes are to set the value of the
	/// boost::program_options::options_description used by this class.
	virtual void setOptionsDescriptions() = 0;

	/// \brief Parse the options
	/// 
	/// This function is to be implemented by any derived class. This function
	/// shall always begin with:
	///     boost::program_options::variables_map vals;
	///     if (!_parseOptions(vals)) {
	///         return;
	///     }
	/// After this, vals can be parsed according to the derived class'
	/// requirements (ie. If an option is required, or optional, is up to
	/// the derived class to enforce by setting _error accordingly).
	virtual void parseOptions() = 0;

	/// \brief Read the options from the configuration file
	/// \param vals Where to put the configuration values
	///
	/// This function is to be called from parseOptions() in a derived class.
	/// See parseOptions() for details.
	bool _parseOptions(boost::program_options::variables_map &vals);

        /// Do we want to allow unregistered field in the config file. default false
        bool allowUnregistered;

	/// The name of the configuration file
	const boost::filesystem::path fileName;

	/// The error status
	bool _error;

	/// The error message
	std::string _errorMsg;

	/// The description of valid configuration options
	boost::program_options::options_description options;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
