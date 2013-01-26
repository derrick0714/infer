#ifndef BASE64_H
#define BASE64_H

#include <string>

namespace vn {
namespace arl {
namespace shared {

namespace Base64 {
	std::string encode(const char *bytes_to_encode, unsigned int in_len);

	std::string decode(const std::string &str);
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
