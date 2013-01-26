/* 
 * File:   DNSResponseEntry.h
 * Author: mike
 *
 * Created on March 20, 2010, 7:54 PM
 */

#ifndef _DNSRESPONSEENTRY_H
#define	_DNSRESPONSEENTRY_H

#include <vector>
#include <string>

#include <boost/asio/ip/address.hpp>

namespace vn {
    namespace arl {
        namespace shared {

            using namespace std;

            class DNSResponseEntry {
            public:
                DNSResponseEntry();
                DNSResponseEntry(const DNSResponseEntry& orig);
                virtual ~DNSResponseEntry();

                void assign(unsigned short _type, unsigned short _klass, unsigned short _ttl,
                            unsigned char _rdlen, unsigned char* _data, const vector<string>& _labels);

                unsigned short getType() const {
                    return type;
                }

                unsigned short getClass() const {
                    return klass;
                }

                unsigned int getTTL() const {
                    return ttl;
                }

                unsigned short getDataLength() const {
                    return rdlen;
                }

                const unsigned char* getData() const {
                    return data;
                }

                const vector<string>& getLabels() const {
                    return labels;
                }

                bool fillAddress(boost::asio::ip::address& dest) const;

            private:

                unsigned short type;
                unsigned short klass;
                unsigned int ttl;
                unsigned short rdlen;
                unsigned char* data;
                vector<string> labels;
            };
        }
    }
}

#endif	/* _DNSRESPONSEENTRY_H */

