/* 
 * File:   IPZone.h
 * Author: Mike
 *
 * Created on January 16, 2010, 3:55 PM
 *
 * This class returns a list of subnets. Generally used for representing IP's
 * or subnets of interest.
 */

#ifndef _IPZONE_H
#define	_IPZONE_H

#include "IPsubnet.h"

#include <vector>

namespace vn {
    namespace arl {
        namespace symptom {

            class IPZone {
            public:
                /// \brief adds a list of subnets from the string to the object list.
                ///        The format is a list of space seperated IPsubnet strings.
                const IPsubnet& addAddressRangeList(const string& spec);

                /// \brief refer to IPsubnet, same functionality on the list.
                bool inRange(const boost::asio::ip::address& check) const;

                /// \brief refer to IPsubnet, same functionality on the list.
                bool inRange(const boost::asio::ip::address_v4& check) const;

                /// \brief refer to IPsubnet, same functionality on the list.
                bool inRange(const boost::asio::ip::address_v6& check) const;

                /// \brief empty the list.
                void clear();

                /// \brief how many IPsubnet objects are on the list.
                int getEntryCount() const;
                
            private:
                std::vector<IPsubnet> ip_list;
            };
        }
    }
}

#endif	/* _IPZONE_H */

