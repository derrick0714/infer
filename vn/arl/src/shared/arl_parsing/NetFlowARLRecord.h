#ifndef NETFLOW_RECORD_H
#define NETFLOW_RECORD_H

#include <string>
#include <arpa/inet.h>
#include <arl/arl_types.h>

#include "../Serializable.hpp"

#include <boost/asio/ip/address.hpp>

// Just some helper macros.
#define NTOHL(D) D = ntohl(D)
#define NTOHS(D) D = ntohs(D)
#define HTONL(D) D = htonl(D)
#define HTONS(D) D = htons(D)

namespace vn {
    namespace arl {
        namespace shared {

            /// \brief This is not the usual NetFlow, it is customized for ARL. Every hour traffic
            ///     recording resets and new entries are created. So each NetFlow represents
            ///     activity on a specific FlowID for one hour. All the data is aggrigated into
            ///     a single entry. The entry is not created until the FlowID was first seen in
            ///     the hour. ARL has 3 version so of the flow record. We should never see version 1
            ///     but we can parse it. Version 2 is the most likely, version 3 is not yet ready.
            ///     For this reason this listing focuses on version 2.
            /// 
            /// Struct:
            ///     
            ///     typedef struct {
            ///         uint32_t first_secs; /* Start time since epoch in seconds */
            ///         uint32_t first_usecs; /* Start time since epoch in microseconds */
            ///         uint32_t last_secs; /* End time since epoch in seconds */
            ///         uint32_t last_usecs; /* End time since epoch in microseconds */
            ///         /* if top 96 bits equal 0-address is IPv4 uint32_t */
            ///         in_addr6_t c_ip; /* Client IP */
            ///         in_addr6_t s_ip; /* Server IP */
            ///         uint32_t c_packets; /* Packets sent by client */
            ///         uint32_t s_packets; /* Packets sent by server */
            ///         uint32_t c_bytes; /* Bytes sent by client */
            ///         uint32_t s_bytes; /* Bytes sent by server */
            ///         uint32_t c_data_bytes; /* Data bytes (layer 7) sent by client */
            ///         uint32_t s_data_bytes; /* Data bytes (Layer 7) sent by server */
            ///         session_flags_t session_flags; /* Flags dealing with the bidirectional session */
            ///         flow_flags_t c_flow_flags; /* Client flow flags */
            ///         flow_flags_t s_flow_flags; /* Server flow flags */
            ///         uint16_t c_port; /* Client port */
            ///         uint16_t s_port; /* Server port */
            ///         uint16_t c_ether_type; /* Client ethernet packet (layer 3) type */
            ///         uint16_t s_ether_type; /* Server ethernet packet (layer 3) type */
            ///         uint16_t c_window_size; /* Client Window size if TCP */
            ///         /* Capsule encryption if GRE & !GRE_IP */
            ///         uint16_t s_window_size; /* Server Window size if TCP */
            ///         /* Capsule encryption if GRE & !GRE_IP */
            ///         uint8_t check_xor; /* Record check XOR byte */
            ///         uint8_t check_char; /* Record check character 'U' (0x55) */
            ///         uint16_t reserved; /* Reserved for future expansion */
            ///     } __attribute__((__packed__)) flow_v2_record_t;

            class NetFlowARLRecord : public Serializable <NetFlowARLRecord> {
                /// \brief A typedef for the size_type of the serialized data
                typedef std::string::size_type size_type;

            public:
                /// \brief Starting the count at 1 to match the file contents.
                enum NetFlowVersion {
                    NetFlow_v1 = 1, NetFlow_v2, NetFlow_v3
                };

            private:
                const NetFlowVersion version;
                TimeStamp start_time;
                TimeStamp last_time;

            protected:
                /// \brief picked sizeof(flow_v3_record_t) because it's the
                ///        largest of the three.
                char data[sizeof(flow_v3_record_t)];

                virtual bool continue_serialize(char* dest) const;

                virtual bool continue_unserialize(char* data);

            public:
                NetFlowARLRecord(NetFlowVersion ver);

                /// \brief Virtual destructor
                virtual ~NetFlowARLRecord();

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

                virtual const NetFlowVersion getVersion() const;
                
                /// \brief Client is determined by the port number (higher port) or by who initiated the
                ///         connection. IPv4 and IPv6 are supported, returns in the Boost address class.
                ///         One can also use getClientIPv6() to get a pointer to in_addr6_t struct, both 
                ///         IPv4 and v6 are represented.
                virtual const boost::asio::ip::address getClientIP() const;
                
                virtual const boost::asio::ip::address getServerIP() const;

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

                /// \returns Client side "flow index" (condensed timestamp)
                virtual const uint32_t getClientPDXid() const;

                /// \returns Server side "flow index" (condensed timestamp)
                virtual const uint32_t getServerPDXid() const;

                /// \returns hashed 3 tuple (ip's and server port)
                virtual const uint32_t getTupleHash() const;

                /// \brief Returns a pointer to a structure with various session information:
                ///     typedef struct {
                ///         uint32_t session_id; /* session ID to tie encapsulated flows */
                ///         uint16_t l3_protocol; /* OSI layer 3 protocol (From ethernet packet) */
                ///         uint16_t reserved;
                ///         uint8_t flags;
                ///         uint8_t layer_3; /* enum layer_3_packet_type_t */
                ///         uint8_t ip_proto_1; /* 1st IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...) */
                ///         uint8_t ip_proto_2; /* Last IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...) */
                ///     } session_flags_t; 
                /// \returns Flags dealing with the bidirectional session
                virtual const session_flags_t* getSessionFlags() const;

                ///    \brief Returns a pointer to the structure with various flag for the client and the server.
                ///         Even though it's named 'flags' there is more information is there, including some
                ///         data from the connection
                ///     typedef struct {
                ///         uint8_t data[16]; /* First 16 bytes of 1st packet (or 1st PUSH if TCP) */
                ///         uint16_t tcp_flags_1; /* TCP flags for 1st packet */
                ///         uint16_t tcp_flags_all; /* TCP flags or'ed together (0 for non-TCP) */
                ///         uint8_t ttl; /* TCP-time to live (hops)--ICMP "type" */
                ///         uint8_t tos; /* TIP-type of service --ICMP "code" */
                ///     } flow_flags_t;
                /// \returns Client flow flags
                virtual const flow_flags_t* getClientFlowFlags() const;

                /// \returns Server flow flags
                virtual const flow_flags_t* getServerFlowFlags() const;

                /// \returns Client port
                virtual const uint16_t getClientPort() const;

                /// \returns Server port
                virtual const uint16_t getServerPort() const;

                /// \returns Client ethernet packet (layer 3) type
                virtual const uint16_t getClientEthernetType() const;

                /// \returns Server ethernet packet (layer 3) type
                virtual const uint16_t getServerEthernetType() const;

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

#endif
