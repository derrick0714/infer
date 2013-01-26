#ifndef INFER_INCLUDE_GETHTTPDATA_H_
#define INFER_INCLUDE_GETHTTPDATA_H_

#include <sstream>

#include <openssl/bio.h>

bool getHTTPData(const char *host, const uint16_t port,
                 const char *fileName, std::stringstream &data);

#endif
