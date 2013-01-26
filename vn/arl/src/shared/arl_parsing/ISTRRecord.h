/* 
 * File:   ISTRRecord.h
 * Author: Mike
 *
 * Created on August 12, 2009, 12:29 AM
 */

#ifndef _ISTRRECORD_H
#define	_ISTRRECORD_H

#include "../Serializable.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/ip/address.hpp>

#include <vector>
#include <string>
#include <iostream>

namespace vn {
    namespace arl {
        namespace shared {

            /// \brief This record is much more free form compared to the others. It is aimed to be readable
            ///     by an analyst. What it provides is a view of the ASCII text being send over TCP connections.
            ///     ARL equates it to running the 'strings' command on a TCP/UDP. While it's mostly true
            ///     there are some restrictions. For example, if the string is too long it will be cut off.
            ///     Think Base64 encoded email attachment. This records are generated for only specific port
            ///     but analyst determined ports (i.e. 25, 53, 80, etc). NOTE: this is the only place to
            ///     find port 80 traffic. This record is also used for the getting DNS informations.
            /// 
            ///     There is no ARL structure for this type. The records are saved in an ASCII file and are
            ///     parsed by the ISTRRecord class (yes, 2 R's)

            class ISTRRecord : public Serializable <ISTRRecord> {
            private:
                std::vector<std::string> lines;
                bool hasHeader;

                boost::asio::ip::address src_ip;
                boost::asio::ip::address dst_ip;
                int src_port;
                int dst_port;
                TimeStamp timeStamp;
                
            public:
                typedef std::string::size_type size_type;
                
                ISTRRecord();

                virtual ~ISTRRecord();

                /// \brief Get the start time of the data
                /// \returns the start time of the data
                virtual TimeStamp startTime() const;

                /// \brief Get the end time of the data
                /// \returns the end time of the data
                virtual TimeStamp endTime() const;

                /// \brief Get the size of the the serialized data
                /// \returns the size of the serialized data
                virtual size_type size() const;

                /// \brief Serialize data
                /// \param ostr the string in which to store the serialized data
                /// \returns true if the data was successfully serialized into ostr
                virtual bool serialize(std::string &ostr) const;

                /// \brief Unserialize data
                /// \param istr the string from which unserialize data
                /// \returns true if the data was successfully unserialized from ostr
                virtual bool unserialize(const std::string &istr);

                virtual const std::vector<std::string>& getLines() const;

                virtual const boost::asio::ip::address& getSourceIP() const;

                virtual const boost::asio::ip::address& getDestinationIP() const;

                virtual const int getSourcePort() const;

                virtual const int getDestinationPort() const;

                virtual const TimeStamp& getTimeStamp() const;
            };

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif	/* _ISTRRECORD_H */

