#ifndef QUERY_MANAGER_CONFIGURATION_H
#define QUERY_MANAGER_CONFIGURATION_H

#include <vector>
#include <string>

namespace ims {

struct QueryManagerConfiguration {
    std::vector <std::string> dbHomes;
    std::string sockPath;
    size_t sockTimeout;
};

} // namespace ims

#endif
