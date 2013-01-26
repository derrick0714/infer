#ifndef TEMPWRITERHELPERS_H
#define TEMPWRITERHELPERS_H

#include <string>

namespace vn {
namespace arl {
namespace shared {

void append(std::string &flow, const char &data);
    
void append(std::string &flow, const uint8_t &data);
    
void append(std::string &flow, uint16_t &data, bool swap = false);

void append(std::string &flow, const uint16_t &data, bool swap = false);

void append(std::string &flow, const uint32_t &data, bool swap = false);

void append(std::string &flow, uint32_t &data, bool swap = false);

void append(std::string &flow, const std::string &data);

void append(std::string &flow, const int32_t &data, bool swap = false);

void append(std::string &flow, int32_t &data, bool swap = false);

} // namespace shared
} // namespace arl
} // namespace vn

#endif
