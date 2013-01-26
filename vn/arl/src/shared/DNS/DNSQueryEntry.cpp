/* 
 * File:   DNSQueryEntry.cpp
 * Author: mike
 * 
 * Created on March 17, 2010, 6:17 PM
 */

#include "DNSQueryEntry.h"
#include <boost/asio/ip/basic_resolver.hpp>


namespace vn {
    namespace arl {
        namespace shared {
            using namespace std;

            DNSQueryEntry::DNSQueryEntry() {
                
            }

            DNSQueryEntry::DNSQueryEntry(const DNSQueryEntry& orig):
                type(orig.type), klass(orig.klass), labels(orig.labels) {
            }

            void DNSQueryEntry::assign(unsigned short _type, unsigned short _klass, vector<string>& _labels) {
                type = _type;
                klass = _klass;
                labels = _labels;
            }

            unsigned short DNSQueryEntry::getType() const {
                return type;
            }

            unsigned short DNSQueryEntry::getClass() const {
                return klass;
            }

            DNSQueryEntry::~DNSQueryEntry() {
            }

            const vector<string>& DNSQueryEntry::getLabels() const {
                return labels;
            }
        }
    }
}