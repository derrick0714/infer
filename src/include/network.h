#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef __FreeBSD__
  #include <netinet/if_ether.h>
#endif
#ifdef __linux__
  #include <netinet/ether.h>
#endif

std::string textToEthernet(const std::string);
std::string ethernetToText(const char*);
std::string ipToText(uint32_t binaryIP);
uint32_t textToIP(const std::string &textIP);

#endif
