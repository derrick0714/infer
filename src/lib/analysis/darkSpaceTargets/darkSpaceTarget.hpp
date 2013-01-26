#ifndef DARK_SPACE_TARGET_HPP
#define DARK_SPACE_TARGET_HPP

#include <vector>
#include <limits>

class DarkSpaceTarget {
  public:
    int8_t initiator;
    uint32_t numBytes;
    uint32_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    std::vector <uint32_t> content;
    DarkSpaceTarget();
};

DarkSpaceTarget::DarkSpaceTarget() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  numBytes = 0;
  numPackets = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

#endif
