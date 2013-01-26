/*
 * Detects the initiator of a TCP connection based on the times and directions
 * of packets constituting the three-way handshake. It maintains an std::map
 * <TwoWayHostPair, TCPInitiatorStats>. Calling aggregate(const TwoWayHostPair&)
 * will update the TCPInitiatorStats class belonging to TwoWayHostPair with any
 * times and directions of packets constituting the TCP handshake that weren't
 * known before. It can be called up to three times, with all calls after the
 * third being no-ops, due to the fact that a TCP handshake can take no more
 * than three flows. Calling getInitiator(const TwoWayHostPair&) will return one
 * of TEMPORARILY_UNKNOWN_INITIATOR, PERMANENTLY_UNKNOWN_INITIATOR
 * INTERNAL_INITIATOR, or EXTERNAL_INITIATOR. TEMPORARILY_UNKNOWN_INITIATOR
 * signifies that not enough information is available to determine the
 * initiator, and that getInitiator() should be called again after more flows
 * are aggregated. Every other return value results in the state used to
 * determine the initiator being deleted.
 */

#ifndef CONNECTION_INITIATOR_HPP
#define CONNECTION_INITIATOR_HPP

#include <vector>
#include <map>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "hostPair.hpp"

#define TCP_PROTOCOL	6
#define UDP_PROTOCOL	17

#define TEMPORARILY_UNKNOWN_INITIATOR	-1
#define PERMANENTLY_UNKNOWN_INITIATOR	0
#define INTERNAL_INITIATOR		1
#define EXTERNAL_INITIATOR		2

#define UNKNOWN_DIRECTION	0
#define OUTBOUND_DIRECTION	1
#define INBOUND_DIRECTION	2

class IPHostPair {
  public:
    uint32_t internalIP;
    uint32_t externalIP;
    IPHostPair(const uint32_t&, const uint32_t&);
    bool operator<(const IPHostPair&) const;
};

inline IPHostPair::IPHostPair(const uint32_t &_internalIP,
                              const uint32_t &_externalIP) {
  internalIP = _internalIP;
  externalIP = _externalIP;
}

inline bool IPHostPair::operator<(const IPHostPair& right) const {
  if (internalIP != right.internalIP) {
    return (internalIP < right.internalIP);
  }
  else {
    if (externalIP != right.externalIP) {
      return (externalIP < right.externalIP);
    }
    else {
      return false;
    }
  }
}

class TCPInitiatorStats {
  public:
    TimeStamp firstSYNTime;
    TimeStamp firstSYNACKTime;
    TimeStamp firstACKTime;
    uint8_t firstSYNDirection;
    uint8_t firstSYNACKDirection;
    uint8_t firstACKDirection;
    uint8_t numFlows;
    TCPInitiatorStats();
};

inline TCPInitiatorStats::TCPInitiatorStats() {
  numFlows = 0;
  firstSYNDirection = UNKNOWN_DIRECTION;
  firstSYNACKDirection = UNKNOWN_DIRECTION;
  firstACKDirection = UNKNOWN_DIRECTION;
}

class ConnectionInitiators {
  public:
    size_t stateSize() const;
    size_t cacheSize() const;
    ConnectionInitiators();
    ConnectionInitiators(const size_t&,
                         std::vector <std::pair <uint32_t, uint32_t> > *localNetworks);
    void setUDPTimeout(const size_t&);
    void setLocalNetworks(std::vector <std::pair <uint32_t, uint32_t> > *_localNetworks);
    void aggregate(const TwoWayHostPair&, const FlowStats*);
    int8_t getInitiator(const TwoWayHostPair&, const uint32_t &sourceIP);
    void clearUnknown();
  private:
    template <typename key, typename value>
    struct bidirectional_unordered_multimap {
      typedef std::pair <key, value> value_type;
      typedef boost::multi_index::multi_index_container <value_type, boost::multi_index::indexed_by <
                boost::multi_index::hashed_unique <boost::multi_index::member <value_type, key, &value_type::first> >,
                boost::multi_index::ordered_non_unique <boost::multi_index::member <value_type, value, &value_type::second> > > > type;
    };
    typedef bidirectional_unordered_multimap <uint32_t, uint32_t>::type ExternalIPs;
    size_t udpTimeout;
    std::vector <std::pair <uint32_t, uint32_t> > *_localNetworks;
    void aggregateTCP(const TwoWayHostPair&, const FlowStats*);
    void aggregateUDP(const TwoWayHostPair&, const FlowStats*);
    bool copyTCPFlagTime(const uint32_t&, TimeStamp&, const TimeStamp&,
                         uint8_t&);
    int8_t getTCPInitiator(const TwoWayHostPair&);
    int8_t getUDPInitiator(const TwoWayHostPair&, const uint32_t&);
    std::map <TwoWayHostPair, TCPInitiatorStats> tcpInitiatorStats;
    std::map <TwoWayHostPair, int8_t> tcpInitiatorCache;
    std::tr1::unordered_map <uint32_t, ExternalIPs> udpInitiatorStats;
    std::map <IPHostPair, int8_t> udpInitiatorCache;
};

size_t ConnectionInitiators::stateSize() const {
  return tcpInitiatorStats.size();
}

size_t ConnectionInitiators::cacheSize() const {
  return tcpInitiatorCache.size();
}

ConnectionInitiators::ConnectionInitiators() {}

ConnectionInitiators::ConnectionInitiators(const size_t &_udpTimeout,
                                           std::vector <std::pair <uint32_t, uint32_t> > *localNetworks) {
  udpTimeout = _udpTimeout;
  _localNetworks = localNetworks;
}

void ConnectionInitiators::setUDPTimeout(const size_t &_udpTimeout) {
  udpTimeout = _udpTimeout;
}

void ConnectionInitiators::setLocalNetworks(std::vector <std::pair <uint32_t, uint32_t> > *localNetworks) {
  _localNetworks = localNetworks;
}

inline bool ConnectionInitiators::copyTCPFlagTime(const uint32_t &sourceIP,
                                                  TimeStamp &currentTimeStamp,
                                                  const TimeStamp &newTimeStamp,
                                                  uint8_t &direction) {
  if ((!currentTimeStamp.seconds() && !currentTimeStamp.microseconds()) && newTimeStamp.seconds()) {
    currentTimeStamp = newTimeStamp;
    if (isInternal(sourceIP, *_localNetworks)) {
      direction = INTERNAL_INITIATOR;
    }
    else {
      direction = EXTERNAL_INITIATOR;
    }
    return true;
  }
  return false;
}

inline void ConnectionInitiators::aggregate(const TwoWayHostPair &key,
                                            const FlowStats *flowStats) {
  switch (flowStats -> protocol()) {
    case TCP_PROTOCOL:
      aggregateTCP(key, flowStats);
      break;
    case UDP_PROTOCOL:
      aggregateUDP(key, flowStats);
      break;
  }
}

inline void ConnectionInitiators::aggregateTCP(const TwoWayHostPair &key,
                                               const FlowStats *flowStats) {
  std::map <TwoWayHostPair, int8_t>::iterator cacheItr = tcpInitiatorCache.find(key);
  std::map <TwoWayHostPair, TCPInitiatorStats>::iterator keyItr;
  static bool newFlow;
  if (cacheItr == tcpInitiatorCache.end()) {
    keyItr = tcpInitiatorStats.find(key);
    if (keyItr == tcpInitiatorStats.end()) {
      keyItr = tcpInitiatorStats.insert(std::make_pair(key, TCPInitiatorStats())).first;
    }
    if (keyItr -> second.numFlows < 3) {
      newFlow = false;
      if (copyTCPFlagTime(flowStats -> sourceIP(),
                          keyItr -> second.firstSYNTime, flowStats -> firstSYNTime(),
                          keyItr -> second.firstSYNDirection) == true) {
        newFlow = true;
      }
      if (copyTCPFlagTime(flowStats -> sourceIP(),
                          keyItr -> second.firstSYNACKTime,
                          flowStats -> firstSYNACKTime(),
                          keyItr -> second.firstSYNACKDirection) == true) {
        newFlow = true;
      }
      if (copyTCPFlagTime(flowStats -> sourceIP(),
                          keyItr -> second.firstACKTime, flowStats -> firstACKTime(),
                          keyItr -> second.firstACKDirection) == true) {
        newFlow = true;
      }
      if (newFlow == true) {
        ++(keyItr -> second.numFlows);
      }
    }
  }
}

inline void ConnectionInitiators::aggregateUDP(const TwoWayHostPair &key,
                                               const FlowStats *flowStats) {
  IPHostPair hostPair(key.internalIP, key.externalIP);
  std::map <IPHostPair, int8_t>::iterator cacheItr = udpInitiatorCache.find(hostPair);
  std::tr1::unordered_map <uint32_t, ExternalIPs>::iterator internalIPItr;
  boost::multi_index::nth_index_iterator <ExternalIPs, 0>::type externalIPItr;
  if (cacheItr == udpInitiatorCache.end()) {
    internalIPItr = udpInitiatorStats.find(hostPair.internalIP);
    if (internalIPItr == udpInitiatorStats.end()) {
      internalIPItr = udpInitiatorStats.insert(std::make_pair(hostPair.internalIP,
                                                              ExternalIPs())).first;
    }
    else {
      while (!internalIPItr -> second.empty() &&
             internalIPItr -> second.get <1> ().begin() -> second < flowStats -> startTime().seconds() - udpTimeout) {
        internalIPItr -> second.get <1> ().erase(internalIPItr -> second.get <1> ().begin());
      }
    }
    externalIPItr = internalIPItr -> second.get <0> ().find(hostPair.externalIP);
    if (hostPair.externalIP == flowStats -> sourceIP()) {
      if (externalIPItr == internalIPItr -> second.get <0> ().end()) {
        internalIPItr -> second.insert(ExternalIPs::value_type(hostPair.externalIP,
                                                               flowStats -> endTime().seconds()));
      }
      else {
        internalIPItr -> second.get <0> ().replace(externalIPItr,
                                                   ExternalIPs::value_type(hostPair.externalIP,
                                                                           flowStats -> endTime().seconds()));
      }
    }
  }
}

inline int8_t ConnectionInitiators::getInitiator(const TwoWayHostPair &key,
                                                 const uint32_t &sourceIP) {
  switch (key.protocol) {
    case TCP_PROTOCOL:
      return getTCPInitiator(key);
      break;
    case UDP_PROTOCOL:
      return getUDPInitiator(key, sourceIP);
      break;
  }
  /* Not reached. */
  return PERMANENTLY_UNKNOWN_INITIATOR;
}

inline int8_t ConnectionInitiators::getTCPInitiator(const TwoWayHostPair &key) {
  std::map <TwoWayHostPair, int8_t>::iterator cacheItr = tcpInitiatorCache.find(key);
  std::map <TwoWayHostPair, TCPInitiatorStats>::iterator keyItr;
  if (cacheItr != tcpInitiatorCache.end()) {
    return cacheItr -> second;
  }
  keyItr = tcpInitiatorStats.find(key);
  if (keyItr == tcpInitiatorStats.end()) {
    return TEMPORARILY_UNKNOWN_INITIATOR;
  }
  if ((keyItr -> second.firstSYNDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstSYNACKDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstSYNTime < keyItr -> second.firstSYNACKTime) ||
      (keyItr -> second.firstSYNACKDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstACKDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstSYNACKTime < keyItr -> second.firstACKTime) ||
      (keyItr -> second.firstSYNDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstACKDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstSYNTime < keyItr -> second.firstACKTime)) {
    tcpInitiatorStats.erase(keyItr);
    tcpInitiatorCache.insert(std::make_pair(key, INTERNAL_INITIATOR));
    return INTERNAL_INITIATOR;
  }
  if ((keyItr -> second.firstSYNDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstSYNACKDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstSYNTime < keyItr -> second.firstSYNACKTime) ||
      (keyItr -> second.firstSYNACKDirection == OUTBOUND_DIRECTION &&
       keyItr -> second.firstACKDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstSYNACKTime < keyItr -> second.firstACKTime) ||
      (keyItr -> second.firstSYNDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstACKDirection == INBOUND_DIRECTION &&
       keyItr -> second.firstSYNTime < keyItr -> second.firstACKTime)) {
    tcpInitiatorStats.erase(keyItr);
    tcpInitiatorCache.insert(std::make_pair(key, EXTERNAL_INITIATOR));
    return EXTERNAL_INITIATOR;
  }
  if (keyItr -> second.numFlows < 3) {
    return TEMPORARILY_UNKNOWN_INITIATOR;
  }
  tcpInitiatorStats.erase(keyItr);
  tcpInitiatorCache.insert(std::make_pair(key, PERMANENTLY_UNKNOWN_INITIATOR));
  return PERMANENTLY_UNKNOWN_INITIATOR;
}

inline void ConnectionInitiators::clearUnknown() {
  tcpInitiatorStats.clear();
  udpInitiatorStats.clear();
}

inline int8_t ConnectionInitiators::getUDPInitiator(const TwoWayHostPair &_hostPair,
                                                    const uint32_t &sourceIP) {
  IPHostPair hostPair(_hostPair.internalIP, _hostPair.externalIP);
  std::map <IPHostPair, int8_t>::iterator cacheItr = udpInitiatorCache.find(hostPair);
  std::tr1::unordered_map <uint32_t, ExternalIPs>::iterator internalIPItr;
  boost::multi_index::nth_index_iterator <ExternalIPs, 0>::type externalIPItr;
  if (cacheItr != udpInitiatorCache.end()) {
    return cacheItr -> second;
  }
  else {
    internalIPItr = udpInitiatorStats.find(hostPair.internalIP);
    if (internalIPItr == udpInitiatorStats.end()) {
      if (isInternal(sourceIP, *_localNetworks)) {
        udpInitiatorCache.insert(std::make_pair(hostPair, INTERNAL_INITIATOR));
        return INTERNAL_INITIATOR;
      }
      else {
        udpInitiatorCache.insert(std::make_pair(hostPair,
                                                PERMANENTLY_UNKNOWN_INITIATOR));
        return PERMANENTLY_UNKNOWN_INITIATOR;
      }
    }
    else {
      externalIPItr = internalIPItr -> second.get <0> ().find(hostPair.externalIP);
      if (externalIPItr == internalIPItr -> second.get <0> ().end()) {
        if (isInternal(sourceIP, *_localNetworks)) {
          udpInitiatorCache.insert(std::make_pair(hostPair, INTERNAL_INITIATOR));
          return INTERNAL_INITIATOR;
        }
        else {
          udpInitiatorCache.insert(std::make_pair(hostPair,
                                                  PERMANENTLY_UNKNOWN_INITIATOR));
          return PERMANENTLY_UNKNOWN_INITIATOR;
        }
      }
      else {
        udpInitiatorCache.insert(std::make_pair(hostPair,
                                                PERMANENTLY_UNKNOWN_INITIATOR));
        return PERMANENTLY_UNKNOWN_INITIATOR;
      }
    }
  }
  /* Not reached. */
  return PERMANENTLY_UNKNOWN_INITIATOR;
}

#endif
