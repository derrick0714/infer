#include "packet.h"

void Packet::initialize(const pcap_pkthdr &pcapHeader, const u_char *pcapPacket,
                        Memory <char> &pcapPacketMemory, Locks &locks,
                        const std::vector <std::pair <uint32_t, uint32_t> > &localNetworks) {
  _freeable = false;
  _pcapPacketMemory = &pcapPacketMemory;
  _locks = &locks;
  _time = pcapHeader.ts;
  pcapPacketMemory.lock();
  _packet = pcapPacketMemory.allocate();
  pcapPacketMemory.unlock();
  locks.lock();
  _lock = locks.allocate();
  locks.unlock();
  _size = pcapHeader.len;
  _capturedSize = pcapHeader.caplen;
  memcpy(_packet, pcapPacket, _capturedSize);
  memcpy(_sourceEthernetAddress, ((ether_header*)_packet) -> ether_shost,
         ETHER_ADDR_LEN);
  memcpy(_destinationEthernetAddress, ((ether_header*)_packet) -> ether_dhost,
         ETHER_ADDR_LEN);
  _typeOfService = ((ip*)(_packet + sizeof(ether_header))) -> ip_tos;
  #if BYTE_ORDER == LITTLE_ENDIAN
    if ((ntohs(((ip*)(_packet + sizeof(ether_header))) -> ip_off) & IP_OFFMASK) == 0) {
  #endif
  #if BYTE_ORDER == BIG_ENDIAN
    if (((((ip*)(_packet + sizeof(ether_header))) -> ip_off) & IP_OFFMASK) == 0) {
  #endif
    _fragmented = false;
  }
  else {
    _fragmented = true;
  }
  _ttl = ((ip*)(_packet + sizeof(ether_header))) -> ip_ttl;
  _protocol = ((ip*)(_packet + sizeof(ether_header))) -> ip_p;
  getPayloadSize();
  _payload = _packet + (_capturedSize - _payloadSize);
  _sourceIP = ((ip*)(_packet + sizeof(ether_header))) -> ip_src.s_addr;
  _destinationIP = ((ip*)(_packet + sizeof(ether_header))) -> ip_dst.s_addr;
  #if BYTE_ORDER == LITTLE_ENDIAN
    _sourceIP = ntohl(_sourceIP);
    _destinationIP = ntohl(_destinationIP);
  #endif
  _internalSource = false;
  _internalDestination = false;
  for (size_t index = 0; index < localNetworks.size(); ++index) {
    if (_sourceIP >= localNetworks[index].first && _sourceIP <= localNetworks[index].second) {
      _internalSource = true;
    }
    if (_destinationIP >= localNetworks[index].first && _destinationIP <= localNetworks[index].second) {
      _internalDestination = true;
    }
  }
  #if BYTE_ORDER == LITTLE_ENDIAN
    _sourceIP = htonl(_sourceIP);
    _destinationIP = htonl(_destinationIP);
  #endif
  switch (_protocol) {
    case IPPROTO_ICMP:
      *(uint8_t*)(&_sourcePort) = ((icmphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> icmp_type;
      *(uint8_t*)(&_destinationPort) = ((icmphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> icmp_code;
      break;
    case IPPROTO_TCP:
      _sourcePort = ((tcphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> th_sport;
      _destinationPort = ((tcphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> th_dport;
      _tcpFlags = ((tcphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> th_flags;
      break;
    case IPPROTO_UDP:
      _sourcePort = ((udphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> uh_sport;
      _destinationPort = ((udphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> uh_dport;
      break;
  }
}

int Packet::lock() {
  return pthread_mutex_lock(_lock);
}

int Packet::unlock() {
  return pthread_mutex_unlock(_lock);
}

const bool &Packet::freeable() {
  return _freeable;
}

void Packet::setFreeable() {
  _freeable = true;
}

const size_t &Packet::references() {
  return _references;
}

const void Packet::reference() {
  ++_references;
}

const void Packet::dereference() {
  --_references;
}

const char *Packet::packet() const {
  return _packet;
}

const char *Packet::payload() const {
  return _payload;
}

const TimeStamp &Packet::time() const {
  return _time;
}

const uint16_t &Packet::size() const {
  return _size;
}

const uint16_t &Packet::capturedSize() const {
  return _capturedSize;
}

const uint16_t &Packet::payloadSize() const {
  return _payloadSize;
}

const char *Packet::sourceEthernetAddress() const {
  return _sourceEthernetAddress;
}

const char *Packet::destinationEthernetAddress() const {
  return _destinationEthernetAddress;
}

const uint8_t &Packet::typeOfService() const {
  return _typeOfService;
}

const bool &Packet::fragmented() const {
  return _fragmented;
}

const uint8_t &Packet::ttl() const {
  return _ttl;
}

const uint8_t &Packet::protocol() const {
  return _protocol;
}

const uint32_t &Packet::sourceIP() const {
  return _sourceIP;
}

const uint32_t &Packet::destinationIP() const {
  return _destinationIP;
}

const bool &Packet::internalSource() const {
  return _internalSource;
}

const bool &Packet::internalDestination() const {
  return _internalDestination;
}

const uint8_t &Packet::icmpType() const {
  return *(uint8_t*)(&_sourcePort);
}

const uint8_t &Packet::icmpCode() const {
  return *(uint8_t*)(&_destinationPort);
}

const uint16_t &Packet::sourcePort() const {
  return _sourcePort;
}

const uint16_t &Packet::destinationPort() const {
  return _destinationPort;
}

const uint8_t &Packet::tcpFlags() const {
  return _tcpFlags;
}

void Packet::free() {
  _locks -> lock();
  _locks -> free(_lock);
  _locks -> unlock();
  _pcapPacketMemory -> lock();
  _pcapPacketMemory -> free(_packet);
  _pcapPacketMemory -> unlock();
}

void Packet::getPayloadSize() {
  if (!_fragmented) {
    switch (_protocol) {
      case IPPROTO_ICMP:
        _payloadSize = _capturedSize - (sizeof(ether_header) - sizeof(ip) + sizeof(icmphdr));
        break;
      case IPPROTO_TCP:
        _payloadSize = _capturedSize - sizeof(ether_header) - sizeof(ip) - (((tcphdr*)(_packet + sizeof(ether_header) + sizeof(ip))) -> th_off << 2);
        break;
      case IPPROTO_UDP:
        _payloadSize = _capturedSize - (sizeof(ether_header) + sizeof(ip) + sizeof(udphdr));
        break;
    }
  }
  else {
    _payloadSize = _capturedSize - (sizeof(ether_header) + sizeof(ip));
  }
}
