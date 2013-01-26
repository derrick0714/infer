/* 
 * File:   DNSResponseEntry.cpp
 * Author: mike
 * 
 * Created on March 20, 2010, 7:54 PM
 */

#include "DNSResponseEntry.h"
#include <iostream>

namespace vn {
    namespace arl {
        namespace shared {

            DNSResponseEntry::DNSResponseEntry() {
            }

            DNSResponseEntry::DNSResponseEntry(const DNSResponseEntry& orig):
                type(orig.type), klass(orig.klass), ttl(orig.ttl), rdlen(orig.rdlen),
                data(orig.data), labels(orig.labels) {
            }

            DNSResponseEntry::~DNSResponseEntry() {
            }

            void DNSResponseEntry::assign(unsigned short _type, unsigned short _klass, unsigned short _ttl,
                                          unsigned char _rdlen, unsigned char* _data, const vector<string>& _labels) {
                type = _type;
                klass = _klass;
                ttl = _ttl;
                rdlen = _rdlen;
                data = _data;
                labels = _labels;
            }

            bool DNSResponseEntry::fillAddress(boost::asio::ip::address& dest) const {
                using namespace boost::asio::ip;

                if(klass == 1) {
                    /// Doing by size to conver differnt types of IP records
                    if(rdlen == 4) {
                        // IPv4
                        dest = address(address_v4(*(unsigned int*)data));

                        return true;
                    } else if(rdlen == 16) {
                        boost::array<unsigned char, 16> addBytes;
                        memcpy(addBytes.elems, data, 16);

                        dest = address(address_v6(addBytes));

                        return true;
                    }
                }

                return false;
            }
        }
    }
}