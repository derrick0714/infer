#ifndef NON_DNS_TRAFFIC_HPP
#define NON_DNS_TRAFFIC_HPP

#include <vector>
#include <limits>

#define UNKNOWN_STATUS	0
#define GOOD_STATUS	1
#define BAD_STATUS	2

class NonDNSTraffic {
  public:
    int8_t initiator;
    uint8_t status;
    uint64_t numBytes;
    uint64_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    std::vector <uint32_t> content;
    uint32_t startTime;
    uint32_t endTime;
    NonDNSTraffic();
};

inline NonDNSTraffic::NonDNSTraffic() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  status = UNKNOWN_STATUS;
  numBytes = 0;
  numPackets = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  content.resize(FlowStats::CONTENT_TYPES);
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
}

#endif
