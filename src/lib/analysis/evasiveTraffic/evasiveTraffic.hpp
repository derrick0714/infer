#ifndef EVASIVE_TRAFFIC_HPP
#define EVASIVE_TRAFFIC_HPP

#include <vector>
#include <limits>

class EvasiveTraffic {
  public:
    int8_t initiator;
    uint32_t numBytes;
    uint32_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t numFrags;
    uint8_t minTTL;
    uint8_t maxTTL;
    uint32_t startTime;
    uint32_t endTime;
    std::vector <uint32_t> content;
    EvasiveTraffic();
};

EvasiveTraffic::EvasiveTraffic() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  numBytes = 0;
  numPackets = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  numFrags = 0;
  minTTL = std::numeric_limits <uint8_t>::max();
  maxTTL = 0;
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

#endif
