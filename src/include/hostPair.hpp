#ifndef HOSTPAIR_HPP
#define HOSTPAIR_HPP

#include <cmath>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

#include "FlowStats.hpp"

// FIXME make this use a network/mask, not range
inline bool isInternal(const uint32_t &ip,
                       const std::vector <std::pair <uint32_t, uint32_t> > &localNetworks) {
  for (size_t network = 0; network < localNetworks.size(); ++network) {
    if (ip >= localNetworks[network].first && ip <= localNetworks[network].second) {
      return true;
    }
  }
  return false;
}

inline bool isTCP(const FlowStats *flowStats) {
  return (flowStats -> protocol() == 6);
}

inline bool isUDP(const FlowStats *flowStats) {
  return (flowStats -> protocol() == 17);
}

class OneWayHostPair {
  public:
    uint8_t protocol;
    uint32_t sourceIP;
    uint32_t destinationIP;
    uint16_t sourcePort;
    uint16_t destinationPort;
    inline OneWayHostPair() {};
    inline OneWayHostPair(uint8_t _protocol, uint32_t _sourceIP,
                          uint32_t _destinationIP, uint16_t _sourcePort,
                          uint16_t _destinationPort) {
      protocol = _protocol;
      sourceIP = _sourceIP;
      destinationIP = _destinationIP;
      sourcePort = _sourcePort;
      destinationPort = _destinationPort;
    }
    inline bool operator<(const OneWayHostPair &rhs) const {
      if (protocol != rhs.protocol) {
        return (protocol < rhs.protocol);
      }
      else {
        if (sourceIP != rhs.sourceIP) {
          return (sourceIP < rhs.sourceIP);
        }
        else {
          if (destinationIP != rhs.destinationIP) {
            return (destinationIP < rhs.destinationIP);
          }
          else {
            if (sourcePort != rhs.sourcePort) {
              return (sourcePort < rhs.sourcePort);
            }
            else {
              if (sourcePort != rhs.sourcePort) {
                return (sourcePort < rhs.sourcePort);
              }
              else {
                return false;
              }
            }
          }
        }
      }
    }
};

class TwoWayHostPair {
  public:
    uint8_t protocol;
    uint32_t internalIP;
    uint32_t externalIP;
    uint16_t internalPort;
    uint16_t externalPort;
    inline TwoWayHostPair() {};
    inline TwoWayHostPair(uint8_t _protocol, uint32_t _internalIP,
                          uint32_t _externalIP, uint16_t _internalPort,
                          uint16_t _externalPort) {
      protocol = _protocol;
      internalIP = _internalIP;
      externalIP = _externalIP;
      internalPort = _internalPort;
      externalPort = _externalPort;
    }
    inline bool operator<(const TwoWayHostPair &rhs) const {
      if (protocol != rhs.protocol) {
        return (protocol < rhs.protocol);
      }
      else {
        if (internalIP != rhs.internalIP) {
          return (internalIP < rhs.internalIP);
        }
        else {
          if (externalIP != rhs.externalIP) {
            return (externalIP < rhs.externalIP);
          }
          else {
            if (internalPort != rhs.internalPort) {
              return (internalPort < rhs.internalPort);
            }
            else {
              if (externalPort != rhs.externalPort) {
                return (externalPort < rhs.externalPort);
              }
              else {
                return false;
              }
            }
          }
        }
      }
    }
} __attribute__ ((packed));

inline uint32_t getInternalIP(const FlowStats *flowStats,
                              const std::vector <std::pair <uint32_t, uint32_t> > &localNetworks) {
  if (isInternal(flowStats -> sourceIP(), localNetworks)) {
    return flowStats -> sourceIP();
  }
  return flowStats -> destinationIP();
}

inline OneWayHostPair makeOneWayHostPair(const FlowStats *flowStats) {
  return OneWayHostPair(flowStats -> protocol(), flowStats -> sourceIP(),
                        flowStats -> destinationIP(), flowStats -> sourcePort(),
                        flowStats -> destinationPort());
}

inline TwoWayHostPair makeTwoWayHostPair(const FlowStats *flowStats,
                                         const std::vector <std::pair <uint32_t, uint32_t> > &localNetworks) {
  if (isInternal(flowStats -> sourceIP(), localNetworks)) {
    return TwoWayHostPair(flowStats -> protocol(), flowStats -> sourceIP(),
                          flowStats -> destinationIP(), flowStats -> sourcePort(),
                          flowStats -> destinationPort());
  }
  return TwoWayHostPair(flowStats -> protocol(), flowStats -> destinationIP(),
                        flowStats -> sourceIP(), flowStats -> destinationPort(),
                        flowStats -> sourcePort());
}

std::string ntop(uint32_t numericAddress, bool swap = true) {
  in_addr numericAddressStruct;
  char presentationAddress[16];
  if (swap) {
    numericAddressStruct.s_addr = ntohl(numericAddress);
  }
  else {
    numericAddressStruct.s_addr = numericAddress;
  }
  inet_ntop(AF_INET, &numericAddressStruct.s_addr, presentationAddress, 16);
  return presentationAddress;
}

uint32_t pton(const std::string &presentationAddress) {
  in_addr numericAddress;
  inet_pton(AF_INET, presentationAddress.c_str(), &numericAddress);
  return ntohl(numericAddress.s_addr);
}

std::vector <uint32_t> cidrToIPs(const std::string network) {
  std::vector <uint32_t> IPs;
  size_t slash = network.find('/');
  uint32_t firstIP = pton(network.substr(0, slash));
  uint32_t lastIP = firstIP + (uint32_t)pow((double)2,
                                            (double)(32 - strtoul(network.substr(slash + 1).c_str(),
                                                                  NULL, 10))) - 1;
  for (size_t ip = firstIP; ip <= lastIP; ++ip) {
    IPs.push_back(ip);
  }
  return IPs;
}

#endif
