#include "FilesystemHelpers.h"

namespace vn {
namespace arl {
namespace shared {

bool mkdir_p(const boost::filesystem::path &p) {
	if (boost::filesystem::is_directory(p)) {
		return true;
	} else if (!mkdir_p(p.parent_path())) {
		return false;
	}

	return boost::filesystem::create_directory(p);
}

} // namespace shared
} // namespace arl
} // namespace vn
