/* 
 * File:   DNSQueryEntry.h
 * Author: mike
 *
 * Created on March 17, 2010, 6:17 PM
 */

#ifndef _DNSQUERYENTRY_H
#define	_DNSQUERYENTRY_H

#include <vector>
#include <string>

namespace vn {
    namespace arl {
        namespace shared {

            using namespace std;

            class DNSQueryEntry {
            public:
                DNSQueryEntry();
                DNSQueryEntry(unsigned char * data);
                DNSQueryEntry(const DNSQueryEntry& orig);
                virtual ~DNSQueryEntry();

                void assign(unsigned short _type, unsigned short _klass, vector<string>& _labels);

                unsigned short getType() const;

                unsigned short getClass() const;

                const vector<string>& getLabels() const;
            private:

                vector<string> labels;
                unsigned short type;
                unsigned short klass;
            };

        }
    }
}

#endif	/* _DNSQUERYENTRY_H */

