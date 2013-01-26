/* arl_file_formats.h
 *
 * Packet index reference
 * NOTE: ALL multibyte references are in network order.
 * 06-19-08 JAW Initial
 *
 * By John Allen Wittkamper (jwittkamper@arl.army.mil)
 *--------------------------------------------------------------*/
#ifndef arl_file_formats_h_
#define arl_file_formats_h_ 1
#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "md5.h"
#include "inet_defs.h"  	/* in_addr6_t, HEADER_OFFSET, SOCKET */
//#include "arl_file_formats.h"		/* arl_file_hdr_t, pidx_data_t, flow_stat_t, flow_v3_record_t */
#define FLOW_VERSION 2
#ifndef ERRNO_MY_ERRORS
#define ERRNO_MY_ERRORS 0x400
#endif	/* ERRNO_MY_ERRORS */
#define EPHEMERAL_PORT	0xFFFE /* Will indicate the ephemeral port */
#define BASE_FNAME_SIZE	sizeof("20090421.24")
#define BASE_PLUS_FNAME_SIZE	sizeof("20090421.24.ftrm")
/*--------------------------------------------------------------
 * This is the for the "condensed timestamp" used in pidx & flow id's
 */
typedef union
{
	struct 
	{ 
		uint32_t hour:12;		// 1 hour (minutes & seconds)
		uint32_t usecond:20;	// microsecond part of date
	}f;
	uint32_t i;
}con_ts_t;						// (4) "condensed" timestamp
#define C_PIDX_MAX	((uint32_t)0xE0FF423F)	/* Largest VALID condensed pidx (59:59.999999) */
#define C_PIDX_ERR	((uint32_t)0xE0FF4240)	/* Condensed pidx error */
/*--------------------------------------------------------------
 * This is a flow ID. 
 * Note: 1 of those fields may be 0
 */
typedef struct
{
	con_ts_t c_time;				// (4) Client side "flow index" (condensed timestamp)
	con_ts_t s_time;				// (4) Server side "flow index" (condensed timestamp)
} flow_id_t;
/*--------------------------------------------------------------*/
typedef struct
{
	uint32_t show_end:1;		// show end time
	uint32_t full_date:1;		// type out full (including microseconds)
} arl_flow_flags_t;
/*--------------------------------------------------------------
 * arl file header format
 * This is the format for the file header. Notice that the version
 * number is basically the same as the type of the records.
 * v1 | 4 | 59 | ... | 59 |		// Version 1 - Flow
 * v2 | 4 | 96 | ... | 96 |		// Version 2 - Flow
 * v3 | 16 | 96 | ... | 96 |	// Version 3 - flow
 *    notes: timestamp will be beginning of hour
 *
 * Flow versions 1 & 2 JUST used a 32 bit int, while all the rest use 
 * "arl_file_hdr_t". 
 *
 * NOTE: The fields are all in NETWORK order for portablility.
 *
 * ARL flows use an 32 bit number for versioning/typing, while others use
 * 16 bits. Pure ARL versioning (ARL files) use only 8 bits for ARL
 * versions and the other 8 bits to specify other vendors.
 */
#define FLOW_V_MASK 	0x0000FFFF	/* mask for "version" number */
#define FLOW_VENDOR 	0x0000FF00	/* mask for "vendor" number */
#define ARL_VENDOR  	0x0000		/* ARL formats */
#define NON_FLOW	  	0x0010		/* Not a flow file less than ver 3 */
#define CISCO_VENDOR	0x0100		/* CISCO formats */
typedef union
{
	struct 
	{ 
		uint32_t key:4;				// Will always be 5, for checking
		uint32_t reserved:12;		// Reserved for future expansion
		uint32_t seg_length:16;		// Number of bytes in each segment
	}f;
	uint32_t i;
}hdr_flags_t;   					// (4) flags for all headers
typedef struct
{
	uint32_t version;   			// (4) version/type
#define HDR_VER_FLOW_1		0x1 	// Initial version of flow files
#define HDR_VER_FLOW_2		0x2 	// Version 2 flow files
#define HDR_VER_FLOW_3		0x3 	// Version 3 flow files
#define HDR_VER_FLOW_4		0x4 	// Version 4 flow files
#define HDR_VER_FLOW_5		0x5 	// Version 5 flow files
#define HDR_VER_FLOW_6		0x6 	// Version 6 flow files
#define HDR_VER_RFLOW		0x7 	// "rflow" files
#define HDR_VER_PIDX		0x11	// PIDX (packet index) files
#define HDR_VER_FIID		0x12	// Flow ID - IP id files
	uint32_t sensor_id;  			// (4) Sensor id
	uint32_t base_date;  			// (4) Base date (date-hour part of 1st packet)
	hdr_flags_t flags;  			// (4) flags for all headers
	uint32_t check;		  			// (4) Checksum for reliability & validity
}  __attribute__((__packed__))arl_file_hdr_t;
/*--------------------------------------------------------------
 * This is the structure to maintain reading and writing ARL files.
 * It is the precursor to a unified alr_fopen/close functionality.
 * It was derived from the "flow_stat_t" structure.
 */
typedef struct
{
	arl_file_hdr_t header;			/* File header */
	struct
	{
		uint32_t file_read:1;		/* read entire file */
		uint32_t map_file:1;		/* if reading, put ALL of file into core (as if it were mapped) */
									/* Will be fast on network (big-endian) boxes */
		uint32_t ignore_blk_ck:1;	/* if reading a v2 file, ignore "block checks" */
		uint32_t v1_read_hack:1;	/* if reading a v1 file, make smaller port server */
		uint32_t inuse:1;			/* we have a currently open file */
		uint32_t write:1;			/* write file */
	} flags;
	uint64_t file_size;				/* Size of file */
	uint32_t record_no;				/* Record number */
	int fd;							/* File descriptor */
	void* supp_data;				/* Pointer to a "supportive data" */
	char filename[FILENAME_MAX + 1];
}  arl_file_t;
/* Supplemental flow structure for "arl_file_t" */
typedef struct
{
	uint32_t packets;   			/* Number of flow packets */
	uint32_t hdr_packets;			/* Number of header packets */
	md5_ctx_t md5_context;			/* MD5 context for V1 flows */
	uint8_t md5_digest[MD5_DIGEST];	/* MD5 digest for V1 flows */
	int flow_count;					/* Number of flows left in this header (Cisco) */
	char* read_buf;					/* mmbuf (real or pseudo) for read */
	uint8_t flow_header[128];		/* Holder for current header */
}  supp_flow_t;
/*--------------------------------------------------------------
 * structure defining the current flow file being written /read
 * for flow interface functions
 */
typedef struct
{
	uint32_t version;				/* (4) version/type */
	uint32_t sensor_id;				/* (4) Sensor id */
	uint32_t base_date;				/* (4) Base date (date-hour part of 1st packet)*/
	hdr_flags_t hdr_flags;			/* (4) flags for all headers */
	struct
	{
		uint32_t file_read:1;		/* read entire file */
		uint32_t map_file:1;		/* if reading, put ALL of file into core (as if it were mapped) */
									/* Will be fast on network (big-endian) boxes */
		uint32_t ignore_blk_ck:1;	/* if reading a v2 file, ignore "block checks" */
		uint32_t v1_read_hack:1;	/* if reading a v1 file, make smaller port server */
		uint32_t inuse:1;			/* we have a currently open file */
		uint32_t write:1;			/* write file */
	} flags;
	uint64_t file_size;				/* running size of file */
	uint32_t record_no;				/* Record number */
	uint32_t packets;   			/* Number of flow packets */
	uint32_t hdr_packets;			/* Number of header packets */
	md5_ctx_t md5_context;			/* MD5 context for V1 flows */
	uint8_t md5_digest[MD5_DIGEST];	/* MD5 digest for V1 flows */
	int fd;							/* File descriptor */
	int flow_count;					/* Number of flows left in this header (Cisco) */
	char* read_buf;					/* mmbuf (real or pseudo) for read */
	uint8_t flow_header[128];		/* Holder for current header */
	char filename[FILENAME_MAX + 1];
} flow_stat_t;
/*--------------------------------------------------------------
 * This is the record for "fiid" files (16).
 * The 1st packet will:
 *	1) Add the "packet" to be part of the flow record
 *	2) Connect the "f_time" and "ip_id" to be connected
 * The last packet will connect the total packet size to the "ip_id".
 * Thus to get all the "packets" in a flow, we must first have a list
 * of ip_id's connected to the flows so that when fetching a flow if 
 * it has fragmented packets we fetch all the fragments (using the 
 * ip_id's).
 */
typedef struct
{
	uint32_t f_time;	// (4) condensed flow timestamp
	uint32_t ip_id;		// (4) ID field from ip header (16 bits for IPv4, 32 for IPv6
	uint32_t pkt_len;	// (4) offset (<<3) + bytecount of last packet
} fiid_record_t;
/*--------------------------------------------------------------
 * Eric's original flow format.
 * Although it's bidirectional, he used unidirectional nomenclature
 * (compare to Cisco flows which are unidirectional).
 */
typedef struct
{
	uint32_t first_secs;		/* Start time since epoch in seconds */
	uint32_t first_usecs;		/* Start time since epoch in microseconds */
	uint32_t last_secs;			/* End time since epoch in seconds */
	uint32_t last_usecs;		/* End time since epoch in microseconds */
	uint32_t src_ip;			/* Source IP */
	uint16_t src_port;			/* Source port */
	uint32_t dst_ip;			/* Destination IP */
	uint16_t dst_port;			/* Destination port */
	uint32_t client_packets;	/* Packets sent by client */
	uint32_t server_packets;	/* Packets sent by server */
	uint32_t client_bytes;		/* Bytes sent by client */
	uint32_t client_data_bytes;	/* Data bytes (layer 7) sent by client */
	uint32_t server_bytes;		/* Bytes sent by server */
	uint32_t server_data_bytes;	/* Data bytes (Layer 7) sent by server */
	uint8_t protocol;			/* IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...) */
	uint8_t tcp_flags;			/* Or if all TCP flags (0 for non-TCP) */
	uint16_t window_size;		/* Window size of TCP */
	uint8_t window_changed;		/* Flag if window size changed */
	uint8_t ttl;				/* Time to live */
	uint8_t session_closed;		/* 1 if session considered closed */
//} __attribute__((__packed__)) flow_v1_record_t;
} flow_v1_record_t;
/*--------------------------------------------------------------
 * This will signify the basic format of the "flow". It is 
 * basically the "last" layer 3 but there are exceptions (ie ETHERNET).
 */
typedef enum 
{
	LAYER_3_ETHERNET = 0,
	LAYER_3_IP,
	LAYER_3_IPv6,
	LAYER_3_ARP, 
	LAYER_3_ICMP,
	LAYER_3_ICMPv6,
	LAYER_3_GRE,
	LAYER_3_IPSEC_AH,
	LAYER_3_IPSEC_ESP,
	LAYER_3_PPP,
	LAYER_3_C_HDLC,
	LAYER_3_X1,
	LAYER_3_X2,
	LAYER_3_X3
}layer_3_packet_type_t;
/*--------------------------------------------------------------
 * flags describing a bidirectional session
 * RANT: There is no way to make a reasonable compatabile bitfield between
 *       Solaris and X86, so we must use ugly "#defines".
 */
typedef struct 
{
	uint32_t session_id;				// (4) session ID to tie encapsulated flows
	uint16_t l3_protocol;				// (4) OSI layer 3 protocol (From ethernet packet)
	uint8_t ses_1_flags;				// (1) reserved for special flags
#define SES_1_FLAG_msb         	0x80	// Most significant bit
	struct
	{
		uint8_t lcl_flags:5;
		uint8_t ip_flags:3;				// IP header flags
#define IP_FLAGS_RP				0x4		// ip reserved flag
#define IP_FLAGS_DP				0x2		// ip don't fragment flag
#define IP_FLAGS_MP				0x1		// ip more fragments flag
	} misc_flags;						// (1) flags
	uint8_t flags;						// (1) flags
#define SES_FLAG_FRAG         	0x80	// At least 1 fragmented packet
#define SES_FLAG_S_END_CLIENT	0x40	// Single end is client
#define SES_FLAG_S_END			0x20	// Single ended session
#define SES_FLAG_ENCAPSULATED	0x10	// encapsulated ip session
#define SES_FLAG_ENCRYPTED		0x08	// Data is encrypted
#define SES_FLAG_GRE			0x04	// GRE (Generic Routing Encapsulation)
#define SES_FLAG_CLOSED			0x02	// 1 if session determined closed 
										// A RST or TCP SYN-FIN flags
#define SES_FLAG_C_IS_C			0x01	// 1 if "client" is positively determined
										// Set if recieve a TCP SYN-ACK flags and our port
										// is larger than the destination port.
	uint8_t layer_3;					// (1) enum layer_3_packet_type_t
	uint8_t ip_proto_1;					// (1) 1st IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
	uint8_t ip_proto_2;					// (1) Last IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
} session_flags_t;
/*--------------------------------------------------------------
 * flags describing a flow
 */
typedef struct
{
	uint8_t data[16];			/* First 16 bytes of 1st packet (or 1st PUSH if TCP) */
	uint16_t tcp_flags_1;		/* TCP flags for 1st packet */
	uint16_t tcp_flags_all;		/* TCP flags or'ed together (0 for non-TCP) */
	uint8_t ttl;				/* TCP-time to live (hops)--ICMP "type" */
	uint8_t tos;				/* TIP-type of service --ICMP "code" */
} flow_flags_t;
/*--------------------------------------------------------------
 * This is the format for the Version 2 flow files.
 * ---An explanation (rant) about the check16 field.
 * Due to problems of data integrity (thanks SGI for your quality product)
 * each record will have a "data check" field. They have been selected to be
 * quick, easily implemented and order agnostic. These should not need to be
 * very stringent as even if the 1st record would pass, the odds against each
 * consecutive record passing would pass rise quite quickly.
 * 1) The "check_char" byte will be a character 'U' (0x55) a common alternate
 *    0/1's pattern. This will have several aspects:
 *    a) QUICK and dirty check that even script could do.
 *    b) It will also be a preset for the XOR check.
 * 2) A byte XOR of the entire record.
 * Since this is a bidirectional flow I have done away with the old 
 * unidirectional terms and go to "client-server". Server is usually considered
 * the IP with the LOWEST port number. Also notice that all addresses are 
 * "IPv6" but IPv4 addresses are really just the last 32bit int.
 */
typedef struct
{
	uint32_t first_secs;			/* Start time since epoch in seconds */
	uint32_t first_usecs;			/* Start time since epoch in microseconds */
	uint32_t last_secs;				/* End time since epoch in seconds */
	uint32_t last_usecs;			/* End time since epoch in microseconds */
	/* if top 96 bits equal 0-address is IPv4 uint32_t */
	in_addr6_t c_ip;				/* Client IP */
	in_addr6_t s_ip;				/* Server IP */
	uint32_t c_packets;				/* Packets sent by client */
	uint32_t s_packets;				/* Packets sent by server */
	uint32_t c_bytes;				/* Bytes sent by client */
	uint32_t s_bytes;				/* Bytes sent by server */
	uint32_t c_data_bytes;			/* Data bytes (layer 7) sent by client */
	uint32_t s_data_bytes;			/* Data bytes (Layer 7) sent by server */
	session_flags_t session_flags;	/* Flags dealing with the bidirectional session */
	flow_flags_t c_flow_flags;		/* Client flow flags */
	flow_flags_t s_flow_flags;		/* Server flow flags */
	uint16_t c_port;				/* Client port */
	uint16_t s_port;				/* Server port */
	uint16_t c_ether_type;			/* Client ethernet packet (layer 3) type */
	uint16_t s_ether_type;			/* Server ethernet packet (layer 3) type */
	uint16_t c_window_size;			/* Client Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
	uint16_t s_window_size;			/* Server Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
	uint8_t check_xor;				/* Record check XOR byte */
	uint8_t check_char;				/* Record check character 'U' (0x55) */
	uint16_t reserved;				/* Reserved for future expansion */
}  __attribute__((__packed__))flow_v2_record_t;
#define FLOW_V2_CHECK_CHAR	0x55
#define FLOW_V2_RECORD_SIZE	96
#define ERRNO_V2_XOR_ERR	(ERRNO_MY_ERRORS + 1)
#define ERRNO_V2_CHAR_ERR	(ERRNO_MY_ERRORS + 2)
/*--------------------------------------------------------------
 * This is the format for the Version 3 flow files.
 * Added flow indexes
 * Added 3 tuple hash for searching
 */
typedef struct
{
	uint32_t first_secs;			/* Start time since epoch in seconds */
	uint32_t first_usecs;			/* Start time since epoch in microseconds */
	uint32_t last_secs;				/* End time since epoch in seconds */
	uint32_t last_usecs;			/* End time since epoch in microseconds */
	/* if top 96 bits equal 0-address is IPv4 uint32_t */
	in_addr6_t c_ip;				/* Client IP */
	in_addr6_t s_ip;				/* Server IP */
	uint32_t c_time;				/* (4) client condensed flow timestamp */
	uint32_t s_time;				/* (4) server condensed flow timestamp */
	uint32_t c_packets;				/* Packets sent by client */
	uint32_t s_packets;				/* Packets sent by server */
	uint32_t c_bytes;				/* Bytes sent by client */
	uint32_t s_bytes;				/* Bytes sent by server */
	uint32_t c_data_bytes;			/* Data bytes (layer 7) sent by client */
	uint32_t s_data_bytes;			/* Data bytes (Layer 7) sent by server */
	flow_id_t flow_id;				/** (8) Flow id (.c_time, .s_time-condensed timestamps) */
	uint32_t tuple_hash;			/** hashed 3 tuple (ip's and server port) */
	session_flags_t session_flags;	/* Flags dealing with the bidirectional session */
	flow_flags_t c_flow_flags;		/* Client flow flags */
	flow_flags_t s_flow_flags;		/* Server flow flags */
	uint16_t c_port;				/* Client port */
	uint16_t s_port;				/* Server port */
	uint16_t c_ether_type;			/* Client ethernet packet (layer 3) type */
	uint16_t s_ether_type;			/* Server ethernet packet (layer 3) type */
	uint16_t c_window_size;			/* Client Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
	uint16_t s_window_size;			/* Server Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
	uint8_t check_xor;				/* Record check XOR byte */
	uint8_t check_char;				/* Record check character 'U' (0x55) */
	uint16_t reserved;				/* Reserved for future expansion */
}  __attribute__((__packed__))flow_v3_record_t;
#define FLOW_V3_CHECK_CHAR	0x55
#define FLOW_V3_RECORD_SIZE	104
#define ERRNO_V3_XOR_ERR	(ERRNO_MY_ERRORS + 3)
#define ERRNO_V3_CHAR_ERR	(ERRNO_MY_ERRORS + 4)
/*--------------------------------------------------------------
 * Realtime flow format. This will be generated with ".rflw" extensions.
 * And will be considered version 0x07.
 * This is really less of a flow format (especially since vsnap will generate
 * 1 record per packet) than an abbreviated packet format. We have it so
 * that Cisco V5 flows can be easily generated for usage with "flow tools".
 */
typedef struct
{
	uint32_t time_sec;		/* Current count of seconds since 0000 UTC 1970 */
	uint32_t time_nanosec;	/* Residual nanoseconds since 0000 UTC 1970 */
	uint32_t src_ip;		/* Source IP address */
	uint32_t dest_ip;		/* Destination IP address */
	uint32_t flow_packets;	/* Packets in the flow */
	uint32_t flow_octets;	/* Total number of Layer 3 bytes in the packets of the flow */
	uint16_t src_port;		/* TCP/UDP source port number or equivalent */
	uint16_t dest_port;		/* TCP/UDP destination port number or equivalent */
	uint8_t tcp_flags;		/* TCP flags */
	uint8_t protocol;		/* IP protocol type (for example, TCP = 6; UDP = 17) */
	uint8_t ttl;			/* TCP-time to live (hops)--ICMP "type" */
	uint8_t tos;			/* TIP-type of service --ICMP "code" */
}__attribute__((__packed__)) rflow_record_t;
/*--------------------------------------------------------------
 * Structures defining the timestamp part packet/flow index header.
 * // (time_t) second resolution
 * typedef time_t uint32_t;		// seconds since epoch (Jan 1, 1970)
 * 1 hour = 60 * 60 = 3600 (0x00000E10) - 0xFFFFF000 -12 bits
 * 1 day = 60 * 60 * 24 = 86400 (0x00015180) - 0xFFFE0000 -17 bits
 * 1 micro 10-6 = 0x000F4240 - 0xFFF00000 - 20 bits
 * 1 nano 10-9 = 0x3B9ACA00 - 0xC0000000
 * //A time value that is accurate to the nearest
 * // microsecond but also has a range of years.
 * struct timeval
 * {
 *   __time_t tv_sec;            // Seconds. (uint32_t)
 *   __suseconds_t tv_usec;      // (20 bits) Microseconds. (uint32_t)
 * };
 * // POSIX version with nanosecond resolution.
 * struct timesec
 * {
 *   __time_t tv_sec;            // Seconds. (uint32_t)
 *   uint32_t tv_nsec; 		     // (30 bits) Nanoseconds. (uint32_t)
 * };
 *
 *----------Terms----------
 * timestamp				-timeval struct <8 bytes>
 * base timestamp			-tv_sec with secs & minutes = 0 <uint32_t>
 * condensed timestamp		-sec & min (12 bits) + usecs (20 bits) <uint32_t>
 * Note: given any 2 the 3rd can be derived.
 * FILE timestamp			-base timestamp of first timestamp in file
 * flow RECORD timestamp	-timestamp of first packet in flow 
 *
 * pidx file format
 * arl_header record_1 record_2 ... record_n
 *     where
 *          arl_header=arl_file_hdr_t
 *          record=pidx_data_t
 */
/*--------------------------------------------------------------
 * packet data record  (cross referenced to v3 flow data)
 */
typedef union
{
	struct 
	{ 
		uint32_t client:1;		// source field had largest port
		uint32_t frag:1;		// fragmented packet
		uint32_t rsrvd:3;		// Unused at this time
		uint32_t ip_flags:3;	// IP:flags, (D=don't frag, M=More frag)
		uint32_t ttl:8;			// IP:time to live
		uint32_t data_len:16;	// Number of data bytes (packet payload)
	}f;
	uint32_t i;
}pidx_data_flags_t;				// (4) "offset" timestamp
typedef struct
{
	con_ts_t p_time;			// (4) condensed packet timestamp
	con_ts_t f_time;			// (4) condensed flow timestamp
	uint32_t tuple_hash;		// (4) 5 tuple hash
	pidx_data_flags_t flags;	// (4) data flags
	uint16_t e_port;			// (2) Ephemeral port (higher number)
//	uint16_t data_len;			// (2) Number of bytes (packet size) */
//	uint16_t data_off;			// (2) Number of bytes offset into packet to reach payload */
	uint8_t fld_1;				// (1) ICMP:type
	uint8_t fld_2;				// (1) ICMP:code, TCP:flags
}  __attribute__((__packed__))pidx_data_t;
/*--------------------------------------------------------------
 * Structure defining the packet index file being written.
 */
typedef struct
{
	uint32_t sensor;				// (4) Sensor id
	uint32_t first_secs;			// (4) Start time since epoch in seconds
	uint32_t first_usecs;			// (4) Start time since epoch in microseconds
	union
	{
		struct 
		{ 
			uint32_t tcp_flags:8;	// if tcp, flags
			uint32_t protocol:8;	// ip protocol (TCP = 6, UDP = 17, etc)
			uint32_t trim:1;		// Packet is going to an "trim" file
			uint32_t istr:1;		// Packet is going to an "istr" file
			uint32_t pkt_size:14;	// packet data size
		}f;
		uint32_t i;
	}flags;							// (4) Flags
	/* if top 96 bits equal 0-address is IPv4 uint32_t */
	in_addr6_t src_ip;				// (16) Source IP
	in_addr6_t dst_ip;				// (16) Destination IP
	uint16_t src_port;				// (2) Source port
	uint16_t dst_port;				// (2) Destination port
	uint16_t seg_length;			// (2) Length of following segment
	flow_id_t flow_id;				// (8) Flow id (.c_time, .s_time-condensed timestamps)
									/* (62) */
}  __attribute__((__packed__))pkt_index_t;
/*--------------------------------------------------------------
 * Session record (internal format) for flow records.
 */
typedef struct
{
	uint32_t first_secs;			/* Start time since epoch in seconds */
	uint32_t first_usecs;			/* Start time since epoch in microseconds */
	uint32_t last_secs;				/* End time since epoch in seconds */
	uint32_t last_usecs;			/* End time since epoch in microseconds */
	/* if top 96 bits equal 0-address is IPv4 uint32_t */
	in_addr6_t c_ip;				/* Client IP */
	in_addr6_t s_ip;				/* Server IP */
	uint32_t c_packets;				/* Packets sent by client */
	uint32_t s_packets;				/* Packets sent by server */
	uint32_t c_bytes;				/* Bytes sent by client */
	uint32_t s_bytes;				/* Bytes sent by server */
	uint32_t c_data_bytes;			/* Data bytes (layer 7) sent by client */
	uint32_t s_data_bytes;			/* Data bytes (Layer 7) sent by server */
uint32_t tuple_hash;			/* a 3 tuple hash */
	flow_id_t flow_id;				/** (8) Flow id (.c_time, .s_time-condensed timestamps) */
	session_flags_t session_flags;	/* Flags dealing with the bidirectional session */
	flow_flags_t c_flow_flags;		/* Client flow flags */
	flow_flags_t s_flow_flags;		/* Server flow flags */
	uint16_t c_port;				/* Client port */
	uint16_t s_port;				/* Server port */
	uint16_t c_ether_type;			/* Client ethernet packet (layer 3) type */
	uint16_t s_ether_type;			/* Server ethernet packet (layer 3) type */
	uint16_t c_window_size;			/* Client Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
	uint16_t s_window_size;			/* Server Window size if TCP */
									/* Capsule encryption if GRE & !GRE_IP */
} session_record_t;
/*--------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /* arl_file_formats_h_ */
/*----------------------------EOF-------------------------------*/
// vim: ts=4 sts=4 sw=4

