#include "network.h"

std::string textToEthernet(const std::string textEthernetAddress) {
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

std::string ipToText(uint32_t binaryIP) {
  in_addr _binaryIP;
  char textIP[16];
  _binaryIP.s_addr = binaryIP;
  inet_ntop(AF_INET, &_binaryIP.s_addr, textIP, 16);
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
