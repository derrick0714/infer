#include <fstream>

#include "SynappConfiguration.h"

namespace vn {
namespace arl {
namespace shared {

SynappConfiguration::SynappConfiguration(const boost::filesystem::path &fileName)
	:fileName(fileName),
	 _error(false),
	 _errorMsg(),
	 options(),
        allowUnregistered(false)
{
}

SynappConfiguration::~SynappConfiguration() {
}

const std::string & SynappConfiguration::error() const {
	return _errorMsg;
}

SynappConfiguration::operator bool() const {
	return !_error;
}

bool SynappConfiguration::_parseOptions(boost::program_options::variables_map &vals) {
	std::ifstream ifs(fileName.file_string().c_str());
	if (!ifs.good()) {
		_errorMsg.assign("config file read error");
		_error = true;
		return false;
	}
	try {
		boost::program_options::store
			(boost::program_options::parse_config_file(ifs, options, allowUnregistered),
			 vals);
		boost::program_options::notify(vals);
	} catch (boost::program_options::unknown_option &unknown) {
		_errorMsg.assign(unknown.what());
		_error = true;
	}
	return !_error;
}


} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out, const vn::arl::shared::SynappConfiguration &conf) {
	out << conf.options;
	
	return out;
}
