#include "dns.h"

void getPTRRecords(std::vector <std::string> &ptrRecords, const uint32_t &ip) {
  in_addr internetAddress = { htonl(ip) };
  hostent *hostEntry = gethostbyaddr((const void*)&internetAddress,
                                     sizeof(internetAddress), AF_INET);
  if (hostEntry != NULL) {
    if (strlen(hostEntry -> h_name) == 0) {
      ptrRecords.push_back(".");
    }
    else {
      ptrRecords.push_back(hostEntry -> h_name);
    }
    while (*(hostEntry -> h_aliases) != NULL) {
      if (strlen(*(hostEntry -> h_aliases)) == 0) {
        ptrRecords.push_back(".");
      }
      else {
        ptrRecords.push_back(*(hostEntry -> h_aliases));
      }
      ++(hostEntry -> h_aliases);
    }
  }
  sort(ptrRecords.begin(), ptrRecords.end());
}

std::vector <std::string> getPTRRecords(const uint32_t &ip) {
  std::vector <std::string> ptrRecords;
  getPTRRecords(ptrRecords, ip);
  return ptrRecords;
}
