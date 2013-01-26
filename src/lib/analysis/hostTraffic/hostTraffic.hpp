#ifndef HOST_TRAFFIC_HPP
#define HOST_TRAFFIC_HPP

#include <vector>
#include <limits>

class HostTraffic {
  public:
    uint32_t firstActivityTime;
    uint32_t lastActivityTime;
    uint32_t inboundFlows;
    uint32_t outboundFlows;
    uint32_t inboundPackets;
    uint32_t outboundPackets;
    uint64_t inboundBytes;
    uint64_t outboundBytes;
    std::vector <uint64_t> inboundPortTraffic;
    std::vector <uint32_t> inboundPortFirstActivityTimes;
    std::vector <uint32_t> inboundPortLastActivityTimes;
    std::vector <uint32_t> inboundPortIPs;
    std::vector <uint64_t> outboundPortTraffic;
    std::vector <uint32_t> outboundPortFirstActivityTimes;
    std::vector <uint32_t> outboundPortLastActivityTimes;
    std::vector <uint32_t> outboundPortIPs;
    std::vector <uint32_t> inboundContent;
    std::vector <uint32_t> outboundContent;
    uint32_t firstInboundTime;
    uint32_t lastInboundTime;
    uint32_t firstOutboundTime;
    uint32_t lastOutboundTime;
    uint16_t roles;
    HostTraffic(const size_t &numInterestingPorts);
};
                                   
HostTraffic::HostTraffic(const size_t &numInterestingPorts) {
  firstActivityTime = std::numeric_limits <uint32_t>::max();
  lastActivityTime = 0;
  inboundFlows = 0;
  outboundFlows = 0;
  inboundPackets = 0;
  outboundPackets = 0;
  inboundBytes = 0;
  outboundBytes = 0;
  inboundPortTraffic.resize(numInterestingPorts);
  outboundPortTraffic.resize(numInterestingPorts);
  inboundContent.resize(FlowStats::CONTENT_TYPES);
  outboundContent.resize(FlowStats::CONTENT_TYPES);
  inboundPortFirstActivityTimes.resize(numInterestingPorts,
                                       std::numeric_limits <uint32_t>::max());
  inboundPortLastActivityTimes.resize(numInterestingPorts);
  outboundPortFirstActivityTimes.resize(numInterestingPorts,
                                        std::numeric_limits <uint32_t>::max());
  outboundPortLastActivityTimes.resize(numInterestingPorts);
  inboundPortIPs.resize(numInterestingPorts);
  outboundPortIPs.resize(numInterestingPorts);
  firstInboundTime = std::numeric_limits <uint32_t>::max();
  lastInboundTime = 0;
  firstOutboundTime = std::numeric_limits <uint32_t>::max();
  lastOutboundTime = 0;
  roles = 0;
}

#endif
