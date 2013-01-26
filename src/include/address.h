#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef __FreeBSD__
  #include <netinet/if_ether.h>
#endif
#ifdef __linux__
  #include <netinet/ether.h>
#endif

std::string textToEthernet(const std::string &textEthernetAddress);
std::string ethernetToText(const char *binaryEthernetAddress);
std::string ipToText(const uint32_t &binaryIP);
uint32_t textToIP(const std::string &textIP);
std::pair <uint32_t, uint32_t> cidrToRange(const std::string &cidr);
bool inRanges(std::vector <std::pair <uint32_t, uint32_t> > &ranges,
              const uint32_t &ip);

#endif
