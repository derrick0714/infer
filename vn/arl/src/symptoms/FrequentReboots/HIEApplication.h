/* 
 * File:   HIEApplication.h
 * Author: Mikhail Sosonkin
 *
 * Created on February 4, 2010, 5:08 PM
 */

#ifndef _HIEAPPLICATION_H
#define	_HIEAPPLICATION_H

#include "../../shared/TimeStamp.h"

#include <boost/asio/ip/address.hpp>

#include <string>

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace boost::asio::ip;
            using namespace vn::arl::shared;
            using namespace std;

            class HIEApplication {
            public:
                HIEApplication(const HIEApplication& orig);

                /// \brief constructors for the events in the config file.
                HIEApplication(const address& _addr, int _port);
                HIEApplication(const string& _name, int _port);

                /// \brief constructors for events that happened.
                HIEApplication(const TimeStamp& _timestamp, const address& _addr, int _port);
                HIEApplication(const TimeStamp& _timestamp, const string& _addr, int _port);

                const string& getName() const;

                const TimeStamp& getTimeStamp() const;

                const address& getAddress() const;

                void setAddress(const address& _addr);

                int getPort() const;

                bool operator==(const HIEApplication &rhs) const;

                bool operator!=(const HIEApplication &rhs) const;
            private:

                string name;
                TimeStamp timestamp;
                address addr;
                int port;
            };

        }
    }
}
#endif	/* _HIEAPPLICATION_H */

