#ifndef FILESYSTEMHELPERS_H
#define FILESYSTEMHELPERS_H

#include <boost/filesystem.hpp>

namespace vn {
namespace arl {
namespace shared {

/// \brief mkdir -p
/// \param p the directory to create
///
/// This function is the equivelant of running mkdir -p p from a shell. It
/// creates the directory p, creating intermediate directories as required.
bool mkdir_p(const boost::filesystem::path &p);

} // namespace shared
} // namespace arl
} // namespace vn

#endif
