#ifndef OSTREAMHELPERS_H
#define OSTREAMHELPERS_H

#include <iosfwd>

// use forward declarations instead of #include

namespace vn {
namespace arl {
namespace shared {

class HBFResult;
class HTTP;
class HTTPResult;
class HBFHTTPResult;

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HBFResult &result);

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HTTP &http);

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HTTPResult &result);

std::ostream & operator<<(std::ostream &os,
						  const vn::arl::shared::HBFHTTPResult &result);

#endif
