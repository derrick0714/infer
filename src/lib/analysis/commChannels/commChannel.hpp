#ifndef COMM_CHANNEL_HPP
#define COMM_CHANNEL_HPP

#include <limits>

class CommChannel {
  public:
    int8_t initiator;
    uint32_t numFlows;
    uint32_t numBytes;
    uint32_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    uint32_t content[FlowStats::CONTENT_TYPES];
    CommChannel();
};

inline CommChannel::CommChannel() {
  initiator = TEMPORARILY_UNKNOWN_INITIATOR;
  numFlows = 0;
  numBytes = 0;
  numPackets = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
  bzero((void*)content, sizeof(content));
}

#endif
