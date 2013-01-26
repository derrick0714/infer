#include <time.h>

#include "SynappArguments.h"

namespace vn {
namespace arl {
namespace shared {

SynappArguments::SynappArguments(int argc, char **argv)
	:argc(argc),
	 argv(argv),
	 _error(false),
	 _errorMsg(),
	 arguments("Synapp Arguments")
{
}

SynappArguments::SynappArguments(const SynappArguments &sa)
	:argc(0),
	 argv(NULL),
	 _error(sa._error),
	 _errorMsg(sa._errorMsg),
	 arguments(sa.arguments)
{
}

SynappArguments::~SynappArguments() {
}

const std::string & SynappArguments::error() const {
	return _errorMsg;
}

SynappArguments::operator bool() const {
	return !_error;
}

bool SynappArguments::_parseOptions(boost::program_options::variables_map &vals)
{
	try {
		boost::program_options::store
			(boost::program_options::parse_command_line(argc, argv, arguments),
			 vals);
		boost::program_options::notify(vals);
	} catch (boost::program_options::unknown_option &unknown) {
		_errorMsg.assign(unknown.what());
		_error = true;
	} catch (boost::program_options::invalid_command_line_syntax &syntax) {
		_errorMsg.assign(syntax.what());
		_error = true;
	}
	return !_error;
}

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out, const vn::arl::shared::SynappArguments &args) {
	out << args.arguments;
	
	return out;
}
