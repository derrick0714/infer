#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "memory.hpp"
#include "locks.h"
#include "timeStamp.h"

class Packet {
  public:
    void initialize(const pcap_pkthdr&, const u_char*, Memory <char>&, Locks&,
                    const std::vector <std::pair <uint32_t, uint32_t> >&);
    int lock();
    int unlock();
    const bool &freeable();
    void setFreeable();
    const size_t &references();
    const void reference();
    const void dereference();
    const char *packet() const;
    const char *payload() const;
    const TimeStamp &time() const;
    const char *sourceEthernetAddress() const;
    const char *destinationEthernetAddress() const;
    const uint8_t &typeOfService() const;
    const bool &fragmented() const;
    const uint8_t &ttl() const;
    const uint16_t &capturedSize() const;
    const uint16_t &size() const;
    const uint16_t &payloadSize() const;
    const uint8_t &protocol() const;
    const uint32_t &sourceIP() const;
    const uint32_t &destinationIP() const;
    const bool &internalSource() const;
    const bool &internalDestination() const;
    const uint8_t &icmpType() const;
    const uint8_t &icmpCode() const;
    const uint16_t &sourcePort() const;
    const uint16_t &destinationPort() const;
    const uint8_t &tcpFlags() const;
    void free();
  private:
    bool _freeable;
    size_t _references;
    Memory <char> *_pcapPacketMemory;
    Locks *_locks;
    char *_packet;
    char *_payload;
    TimeStamp _time;
    char _sourceEthernetAddress[ETHER_ADDR_LEN];
    char _destinationEthernetAddress[ETHER_ADDR_LEN];
    uint8_t _typeOfService;
    bool _fragmented;
    uint8_t _ttl;
    uint16_t _size;
    uint16_t _capturedSize;
    uint16_t _payloadSize;
    uint8_t _protocol;
    uint32_t _sourceIP;
    uint32_t _destinationIP;
    bool _internalSource;
    bool _internalDestination;
    uint16_t _sourcePort;
    uint16_t _destinationPort;
    uint8_t _tcpFlags;
    pthread_mutex_t *_lock;
    void getPayloadSize();
};

#endif
