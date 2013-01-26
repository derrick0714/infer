#include "address.h"

std::string textToEthernet(const std::string &textEthernetAddress) {
  ether_addr binaryEthernetAddress;
  ether_aton_r(textEthernetAddress.c_str(), &binaryEthernetAddress);
  #ifdef __FreeBSD__
    return std::string((char*)binaryEthernetAddress.octet, ETHER_ADDR_LEN);
  #endif
  #ifdef __linux__
    return std::string((char*)binaryEthernetAddress.ether_addr_octet,
                       ETHER_ADDR_LEN);
  #endif
}

std::string ethernetToText(const char *binaryEthernetAddress) {
  ether_addr _binaryEthernetAddress;
  char textEthernetAddress[17];
  #ifdef __FreeBSD__
    memcpy(_binaryEthernetAddress.octet, binaryEthernetAddress, ETHER_ADDR_LEN);
  #endif
  #ifdef __linux__
    memcpy(_binaryEthernetAddress.ether_addr_octet, binaryEthernetAddress,
           ETHER_ADDR_LEN);
  #endif
  ether_ntoa_r(&_binaryEthernetAddress, textEthernetAddress);
  return textEthernetAddress;
} 

std::string ipToText(const uint32_t &binaryIP) {
  in_addr _binaryIP;
  char textIP[INET_ADDRSTRLEN];
  _binaryIP.s_addr = binaryIP;
  #if BYTE_ORDER == LITTLE_ENDIAN
    _binaryIP.s_addr = ntohl(_binaryIP.s_addr);
  #endif
  inet_ntop(AF_INET, &_binaryIP.s_addr, textIP, INET_ADDRSTRLEN);
  return textIP;
}

uint32_t textToIP(const std::string &textIP) {
  in_addr binaryIP;
  inet_pton(AF_INET, textIP.c_str(), &binaryIP);
  #if BYTE_ORDER == LITTLE_ENDIAN
    binaryIP.s_addr = ntohl(binaryIP.s_addr);
  #endif
  return binaryIP.s_addr;
}

std::pair <uint32_t, uint32_t> cidrToRange(const std::string &network) {
  std::pair <uint32_t, uint32_t> range;
  size_t slash = network.rfind('/');
  uint32_t ip = textToIP(network.substr(0, slash));
  if (slash == std::string::npos) {
    return std::make_pair(ip, ip);
  }
  return std::make_pair(ip,
                        ip + pow((double)2,
                                 (double)(32 - strtoul(network.substr(slash + 1).c_str(),
                                                       NULL, 10))) - 1);
}

bool inRanges(std::vector <std::pair <uint32_t, uint32_t> > &ranges,
              const uint32_t &ip) {
  for (size_t index = 0; index < ranges.size(); ++index) {
    if (ip >= ranges[index].first && ip <= ranges[index].second) {
      return true;
    }
  }
  return false;
}
