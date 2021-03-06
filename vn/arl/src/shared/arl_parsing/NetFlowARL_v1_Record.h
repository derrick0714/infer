/* 
 * File:   NetFlowARL_v1_Record.h
 * Author: Mike
 *
 * Created on August 3, 2009, 10:56 PM
 */

#ifndef _NETFLOWARL_V1_RECORD_H
#define	_NETFLOWARL_V1_RECORD_H

#include "NetFlowARLRecord.h"

namespace vn {
    namespace arl {
        namespace shared {

            class NetFlowARL_v1_Record : public NetFlowARLRecord {
                /// \brief A typedef for the size_type of the serialized data
                typedef std::string::size_type size_type;

            private:
                in_addr6_t client_ip;
                in_addr6_t server_ip;

            protected:
                virtual bool continue_unserialize(char* data);

                virtual bool continue_serialize(char* dest) const;

            public:
                NetFlowARL_v1_Record();
                virtual ~NetFlowARL_v1_Record();

                /// \brief Get the size of the the serialized data
                /// \returns the size of the serialized data
                virtual size_type size() const;

                /// \returns Client IP
                virtual const in_addr6_t* getClientIPv6() const;

                /// \returns Server IP
                virtual const in_addr6_t* getServerIPv6() const;

                /// \returns Packets sent by client
                virtual const uint32_t getClientPacketCount() const;

                /// \returns Packets sent by server
                virtual const uint32_t getServerPacketCount() const;

                /// \returns Bytes sent by client
                virtual const uint32_t getClientByteCount() const;

                /// \returns Bytes sent by server
                virtual const uint32_t getServerByteCount() const;

                /// \returns Data bytes (layer 7) sent by client
                virtual const uint32_t getClientDataCount() const;

                /// \returns Data bytes (Layer 7) sent by server
                virtual const uint32_t getServerDataCount() const;

                /// \returns Client port
                virtual const uint16_t getClientPort() const;

                /// \returns Server port
                virtual const uint16_t getServerPort() const;

                /// \returns Client Window size if TCP
                virtual const uint16_t getClientWindowSize() const;

                /// \returns Server Window size if TCP
                virtual const uint16_t getServerWindowSize() const;

                /// \returns IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
                virtual const uint8_t getProtocol() const;

                /// \returns Or if all TCP flags (0 for non-TCP)
                virtual const uint8_t getTcpFlags() const;

                /// \returns Flag if window size changed
                virtual const uint8_t getWindowChanged() const;

                /// \returns Time to live
                virtual const uint8_t getTtl() const;

                /// \returns 1 if session considered closed
                virtual const uint8_t getSessionClosed() const;
            };

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif	/* _NETFLOWARL_V1_RECORD_H */

