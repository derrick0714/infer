/* 
 * File:   IPNetworkMask.h
 * Author: Mike
 *
 * Created on January 16, 2010, 1:37 PM
 *
 * This class is used for representing a subnet and matching IP's to it.
 */

#ifndef _IPNETWORKMASK_H
#define	_IPNETWORKMASK_H

#include <boost/asio/ip/address.hpp>
#include <string>

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace std;

            class IPsubnet {
            public:
                /// \brief Constructor the class. Builds a subnet based on a
                ///        standard specification
                ///
                ///  IP's are acceptable in either IPv4 and IPv6 formats.
                ///    ex: 192.168.1.2/24 or 2001:480:60:55::3/96
                explicit IPsubnet(const string& desc);

                /// \returns the error string
                const string& getErrorStr();

                /// \returns true if error occured during parsing.
                bool errored();

                /// \brief determines if a specific address is in the specified
                ///        subnet. This method is IP version independent;
                bool inRange(const boost::asio::ip::address& check);

                /// \brief determines if a specific address is in the specified
                ///        subnet. This method is IPv4 specific
                bool inRange(const boost::asio::ip::address_v4& check);

                /// \brief determines if a specific address is in the specified
                ///        subnet. This method is IPv6 specific
                bool inRange(const boost::asio::ip::address_v6& check);
                
            private:
                boost::asio::ip::address_v4::bytes_type addr4;
                boost::asio::ip::address_v6::bytes_type addr6;
                string err_str;
                bool isIPv6;
                bool error;
                int bits;

                bool inRange(unsigned char* local, unsigned char* check, int size);
            };

        }
    }
}

#endif	/* _IPNETWORKMASK_H */

