/* 
 * File:   arl_types.h
 * Author: Mike
 *
 * Created on August 3, 2009, 6:59 PM
 *
 * 
 *
 * This file contains C structures for describing ARL files:
 *  PIDX
 *  NetFlow
 *
 * The PIDX file format is an ARL File header (arl_file_hdr_t) followed by
 *  a list of records (pidx_data_t).
 *   arl_file_hdr_t || pidx_data_t || ... || pidx_data_t
 *
 * The NetFlow file format is a version number (4 byte big-endian int)
 *  followed by a list of records (flow_v#_record_t, # is the flow version).
 *   arl_file_hdr_t.version (1 | 2 | 3) ||
 *   flow_v[1 | 2 | 3]_record_t ||
 *   ... ||
 *   flow_v[1 | 2 | 3]_record_t
 */

#ifndef _ARL_TYPES_H
#define	_ARL_TYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define FLOW_V3_CHECK_CHAR	0x55
#define FLOW_V2_CHECK_CHAR	0x55

    /// \brief special condensed version of time. Note that when stored in a file
    ///         it is stored in big-endian (network order) intenger.

    typedef union {

        struct {
            uint32_t hour : 12; /* 1 hour (minutes & seconds) */
            uint32_t usecond : 20; /* microsecond part of date */
        } f;
        uint32_t i;
    } con_ts_t;

    typedef struct {
        con_ts_t c_time; // (4) Client side "flow index" (condensed timestamp)
        con_ts_t s_time; // (4) Server side "flow index" (condensed timestamp)
    } flow_id_t;

    typedef struct {
        uint32_t show_end : 1; /* show end time */
        uint32_t full_date : 1; /* type out full (including microseconds) */
    } arl_flow_flags_t;

    typedef union {

        struct {
            uint32_t key : 4; // Will always be 5, for checking
            uint32_t reserved : 12; // Reserved for future expansion
            uint32_t seg_length : 16; // Number of bytes in each segment
        } f;
    } hdr_flags_t; /* (4) flags for all headers */

    typedef struct {
        uint32_t version; /* (4) version/type */
        uint32_t sensor_id; /* (4) Sensor id */
        uint32_t base_date; /* (4) Base date (date-hour part of 1st packet)*/
        hdr_flags_t flags; /* (4) flags for all headers */
        uint32_t check; // (4) Checksum for reliability & validity

    } __attribute__((__packed__))arl_file_hdr_t;




    // NetFlow Types.
#define SES_1_FLAG_msb              	0x80	// Most significant bit
#define IP_FLAGS_RP			0x4		// ip reserved flag
#define IP_FLAGS_DP			0x2		// ip don't fragment flag
#define IP_FLAGS_MP                 	0x1		// ip more fragments flag
#define SES_FLAG_FRAG                   0x80	// At least 1 fragmented packet
#define SES_FLAG_S_END_CLIENT     	0x40	// Single end is client
#define SES_FLAG_S_END			0x20	// Single ended session
#define SES_FLAG_ENCAPSULATED   	0x10	// encapsulated ip session
#define SES_FLAG_ENCRYPTED		0x08	// Data is encrypted
#define SES_FLAG_GRE			0x04	// GRE (Generic Routing Encapsulation)
#define SES_FLAG_CLOSED			0x02	// 1 if session determined closed 
#define SES_FLAG_C_IS_C			0x01	// 1 if "client" is positively determined

    typedef struct {
        uint32_t session_id; // (4) session ID to tie encapsulated flows
        uint16_t l3_protocol; // (4) OSI layer 3 protocol (From ethernet packet)
        uint8_t ses_1_flags; // (1) reserved for special flags

        struct {
            uint8_t lcl_flags : 5;
            uint8_t ip_flags : 3; // IP header flags
        } misc_flags; // (1) flags
        uint8_t flags; // (1) flags
        // A RST or TCP SYN-FIN flags
        // Set if recieve a TCP SYN-ACK flags and our port
        // is larger than the destination port.
        uint8_t layer_3; // (1) enum layer_3_packet_type_t
        uint8_t ip_proto_1; // (1) 1st IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
        uint8_t ip_proto_2; // (1) Last IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
    } session_flags_t;

    typedef struct {
        uint8_t data[16]; /* First 16 bytes of 1st packet (or 1st PUSH if TCP) */
        uint16_t tcp_flags_1; /* TCP flags for 1st packet */
        uint16_t tcp_flags_all; /* TCP flags or'ed together (0 for non-TCP) */
        uint8_t ttl; /* TCP-time to live (hops)--ICMP "type" */
        uint8_t tos; /* TIP-type of service --ICMP "code" */
    } flow_flags_t;

    struct x_libnet_in6_addr {

        union {
            uint8_t __u6_addr8[16];
            uint16_t __u6_addr16[8];
            uint32_t __u6_addr32[4];
        } __u6_addr; /* 128-bit IP6 address */
    };

    typedef struct x_libnet_in6_addr in_addr6_t;

    typedef struct {
        uint32_t first_secs; /* Start time since epoch in seconds */
        uint32_t first_usecs; /* Start time since epoch in microseconds */
        uint32_t last_secs; /* End time since epoch in seconds */
        uint32_t last_usecs; /* End time since epoch in microseconds */
    } flow_time_sect;

    typedef struct {
        uint32_t first_secs; /* Start time since epoch in seconds */
        uint32_t first_usecs; /* Start time since epoch in microseconds */
        uint32_t last_secs; /* End time since epoch in seconds */
        uint32_t last_usecs; /* End time since epoch in microseconds */
        uint32_t src_ip; /* Source IP */
        uint16_t src_port; /* Source port */
        uint32_t dst_ip; /* Destination IP */
        uint16_t dst_port; /* Destination port */
        uint32_t client_packets; /* Packets sent by client */
        uint32_t server_packets; /* Packets sent by server */
        uint32_t client_bytes; /* Bytes sent by client */
        uint32_t client_data_bytes; /* Data bytes (layer 7) sent by client */
        uint32_t server_bytes; /* Bytes sent by server */
        uint32_t server_data_bytes; /* Data bytes (Layer 7) sent by server */
        uint8_t protocol; /* IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...) */
        uint8_t tcp_flags; /* Or if all TCP flags (0 for non-TCP) */
        uint16_t window_size; /* Window size of TCP */
        uint8_t window_changed; /* Flag if window size changed */
        uint8_t ttl; /* Time to live */
        uint8_t session_closed; /* 1 if session considered closed */
    } flow_v1_record_t;

    typedef struct {
        uint32_t first_secs; /* Start time since epoch in seconds */
        uint32_t first_usecs; /* Start time since epoch in microseconds */
        uint32_t last_secs; /* End time since epoch in seconds */
        uint32_t last_usecs; /* End time since epoch in microseconds */
        /* if top 96 bits equal 0-address is IPv4 uint32_t */
        in_addr6_t c_ip; /* Client IP */
        in_addr6_t s_ip; /* Server IP */
        uint32_t c_packets; /* Packets sent by client */
        uint32_t s_packets; /* Packets sent by server */
        uint32_t c_bytes; /* Bytes sent by client */
        uint32_t s_bytes; /* Bytes sent by server */
        uint32_t c_data_bytes; /* Data bytes (layer 7) sent by client */
        uint32_t s_data_bytes; /* Data bytes (Layer 7) sent by server */
        session_flags_t session_flags; /* Flags dealing with the bidirectional session */
        flow_flags_t c_flow_flags; /* Client flow flags */
        flow_flags_t s_flow_flags; /* Server flow flags */
        uint16_t c_port; /* Client port */
        uint16_t s_port; /* Server port */
        uint16_t c_ether_type; /* Client ethernet packet (layer 3) type */
        uint16_t s_ether_type; /* Server ethernet packet (layer 3) type */
        uint16_t c_window_size; /* Client Window size if TCP */
        /* Capsule encryption if GRE & !GRE_IP */
        uint16_t s_window_size; /* Server Window size if TCP */
        /* Capsule encryption if GRE & !GRE_IP */
        uint8_t check_xor; /* Record check XOR byte */
        uint8_t check_char; /* Record check character 'U' (0x55) */
        uint16_t reserved; /* Reserved for future expansion */
    } __attribute__((__packed__))flow_v2_record_t;

    typedef struct {
        uint32_t first_secs; /* Start time since epoch in seconds */
        uint32_t first_usecs; /* Start time since epoch in microseconds */
        uint32_t last_secs; /* End time since epoch in seconds */
        uint32_t last_usecs; /* End time since epoch in microseconds */
        /* if top 96 bits equal 0-address is IPv4 uint32_t */
        in_addr6_t c_ip; /* Client IP */
        in_addr6_t s_ip; /* Server IP */
        uint32_t c_time; /* (4) client condensed flow timestamp */
        uint32_t s_time; /* (4) server condensed flow timestamp */
        uint32_t c_packets; /* Packets sent by client */
        uint32_t s_packets; /* Packets sent by server */
        uint32_t c_bytes; /* Bytes sent by client */
        uint32_t s_bytes; /* Bytes sent by server */
        uint32_t c_data_bytes; /* Data bytes (layer 7) sent by client */
        uint32_t s_data_bytes; /* Data bytes (Layer 7) sent by server */
        flow_id_t flow_id; /** (8) Flow id (.c_time, .s_time-condensed timestamps) */
        uint32_t tuple_hash; /** hashed 3 tuple (ip's and server port) */
        session_flags_t session_flags; /* Flags dealing with the bidirectional session */
        flow_flags_t c_flow_flags; /* Client flow flags */
        flow_flags_t s_flow_flags; /* Server flow flags */
        uint16_t c_port; /* Client port */
        uint16_t s_port; /* Server port */
        uint16_t c_ether_type; /* Client ethernet packet (layer 3) type */
        uint16_t s_ether_type; /* Server ethernet packet (layer 3) type */
        uint16_t c_window_size; /* Client Window size if TCP */
        /* Capsule encryption if GRE & !GRE_IP */
        uint16_t s_window_size; /* Server Window size if TCP */
        /* Capsule encryption if GRE & !GRE_IP */
        uint8_t check_xor; /* Record check XOR byte */
        uint8_t check_char; /* Record check character 'U' (0x55) */
        uint16_t reserved; /* Reserved for future expansion */
    } __attribute__((__packed__))flow_v3_record_t;



    // pidx types

    typedef union {

        struct {
            uint32_t client : 1; /* source field had largest port */
            uint32_t frag : 1; /* fragmented packet */
            uint32_t rsrvd : 3; /* Unused at this time */
            uint32_t flags : 3; /* IP:flags, (D=don't frag, M=More frag) */
            uint32_t ttl : 8; /* IP:time to live */
            uint32_t data_len : 16; /* Number of data bytes */
        } f;
        uint32_t i;
    } pidx_data_flags_t; /* (4) "offset" timestamp */

    typedef struct {
        con_ts_t p_time; /* (4) condensed packet timestamp */
        con_ts_t f_time; /* (4) condensed flow timestamp */
        uint32_t tuple_hash; /* (4) 5 tuple hash */
        pidx_data_flags_t flags; /* (4) data flags */
        uint16_t port; /* (2) Ephemeral port (higher number) */
        uint8_t fld_1; /* (1) ICMP:type */
        uint8_t fld_2; /* (1) TCP:flags, ICMP:code */
    } __attribute__((__packed__))pidx_data_t;
#ifdef	__cplusplus
}
#endif

#endif	/* _ARL_TYPES_H */

