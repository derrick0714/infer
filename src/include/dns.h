/* DNS-resolution functions. */

#ifndef DNS_H
#define DNS_H

#include <string>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
 * Given a binary IPv4 address in native byte order, looks up its PTR DNS
 * records using the system's resolver facilities, adds them to the ptrRecords
 * vector, and sorts the vector in ascending lexicographical order.
 */
void getPTRRecords(std::vector <std::string> &ptrRecords, const uint32_t &ip);
/* Same as above, but creates its own vector of PTR records and returns it. */
std::vector <std::string> getPTRRecords(const uint32_t &ip);

#endif
