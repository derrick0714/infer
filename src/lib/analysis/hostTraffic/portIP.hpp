#ifndef PORT_IP_HPP
#define PORT_IP_HPP

#include <vector>
#include <limits>

class PortIPHostPair {
  public:
    uint32_t internalIP;
    uint32_t externalIP;
    uint16_t port;
    uint8_t initiator;
    PortIPHostPair();
    PortIPHostPair(const uint32_t &internalIP, const uint32_t &externalIP,
                   const uint16_t &port, const uint8_t &initiator);
    bool operator<(const PortIPHostPair&) const;
};

PortIPHostPair::PortIPHostPair() {}

PortIPHostPair::PortIPHostPair(const uint32_t &_internalIP,
                               const uint32_t &_externalIP,
                               const uint16_t &_port,
                               const uint8_t &_initiator) {
  internalIP = _internalIP;
  externalIP = _externalIP;
  port = _port;
  initiator = _initiator;
}
                                 
bool PortIPHostPair::operator<(const PortIPHostPair &right) const {
  if (internalIP != right.internalIP) {
    return (internalIP < right.internalIP);
  }
  else {
    if (externalIP != right.externalIP) {
      return (externalIP < right.externalIP);
    }
    else {
      if (port != right.port) {
        return (port < right.port);
      }
      else {
        if (initiator != right.initiator) {
          return (initiator < right.initiator);
        }
        else {
          return false;
        }
      }
    }
  }
}

class PortIP {
  public:
    uint32_t numBytes;
    uint32_t numPackets;
    uint16_t minPacketSize;
    uint16_t maxPacketSize;
    uint32_t startTime;
    uint32_t endTime;
    uint16_t asNumber;
    int16_t countryNumber;
    std::vector <uint32_t> content;
    PortIP();
};

PortIP::PortIP() {
  numBytes = 0;
  numPackets = 0;
  minPacketSize = std::numeric_limits <uint16_t>::max();
  maxPacketSize = 0;
  startTime = std::numeric_limits <uint32_t>::max();
  endTime = 0;
  content.resize(FlowStats::CONTENT_TYPES);
}

#endif
