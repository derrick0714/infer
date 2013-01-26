#ifndef INFER_INCLUDE_BASE64_H_
#define INFER_INCLUDE_BASE64_H_

#include <string>

namespace Base64 {
	std::string encode(const char *bytes_to_encode, unsigned int in_len);

	std::string decode(const std::string &str);
}

#endif
