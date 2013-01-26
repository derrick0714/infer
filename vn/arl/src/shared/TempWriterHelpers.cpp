#include <netinet/in.h>

#include "TempWriterHelpers.h"

namespace vn {
namespace arl {
namespace shared {

void append(std::string &flow, const char &data) {
  flow += data;
}
    
void append(std::string &flow, const uint8_t &data) {
  flow += data;
}
    
void append(std::string &flow, uint16_t &data, bool swap) {
  if (swap) {
    data = htons(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

void append(std::string &flow, const uint16_t &data, bool swap) {
  static uint16_t _data;
  if (swap) {
    _data = htons(data);
    flow.append((const char*)&_data, sizeof(data));
  }
  else {
    flow.append((const char*)&data, sizeof(data));
  }
}

void append(std::string &flow, const uint32_t &data, bool swap) {
  static uint32_t _data;
  if (swap) {
    _data = htonl(data);
    flow.append((const char*)&_data, sizeof(_data));
  }
  else {
    flow.append((const char*)&data, sizeof(_data));
  }
}

void append(std::string &flow, uint32_t &data, bool swap) {
  if (swap) {
    data = htonl(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

void append(std::string &flow, const std::string &data) {
  flow += data;
}

void append(std::string &flow, const int32_t &data, bool swap) {
  static int32_t _data;
  if (swap) {
    _data = htonl(data);
    flow.append((const char*)&_data, sizeof(_data));
  }
  else {
    flow.append((const char*)&data, sizeof(_data));
  }
}

void append(std::string &flow, int32_t &data, bool swap) {
  if (swap) {
    data = htonl(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

} // namespace shared
} // namespace arl
} // namespace vn
