#ifndef HOST_PAIR_TRAFFIC_HPP
#define HOST_PAIR_TRAFFIC_HPP

#include <vector>

class HostPairTraffic {
  public:
    int8_t initiator;
    uint32_t numPackets;
    uint32_t numBytes;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    std::vector <uint32_t> content;
    HostPairTraffic();
};

HostPairTraffic::HostPairTraffic() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  numPackets = 0;
  numBytes = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

#endif
