/* inet_defs.h
 *
 * General support routines for inet functionality.
 * OSI Layer                       Protocols
 * 1. Physical     RS232, 10base-x, 802.11, ISDN
 * 2. Link         Ethernet, Wi-Fi, Token ring, PPP, SLIP, FDDI, ATM, DTM, Frame Relay, SMDS
 * 3. Network      IP (IPv4, IPv6), ICMP, IGMP, ARP, RARP, IPX, SNA, GRE
 * 4. Transport    TCP, UDP, NETBUI, SPX, SSL, SCSI
 * 5. Session      XDR, ASN.1, SSH, ZIP, NetBIOS, encryption/decription
 * 6. Application  ASCII, EBCDIC, HTTP, XML,  encryption/decription
 * 7. Application  DNS, TLS/SSL, TFTP, FTP, HTTP, IMAP, IRC, NNTP, POP3,
 * 				SIP, SMTP, SNMP, SSH, TELNET, BitTorrent, RTP, rlogin
 *
 * 06-22-05 JAW Initial
 *
 * Note: I've added some defines that I can't find, they
 * should go away someday!!
 *
 * By John Allen Wittkamper (jwittkamper@gmail.com)
 *--------------------------------------------------------------*/
#ifndef inet_defs_h_
#define inet_defs_h_
#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#include "pcap.h"			/* define pcaplib functions/structures */
#include "libnet.h"
//#include "inet_defs.h"  	/* in_addr6_t, HEADER_OFFSET, SOCKET */
/*--------------------------------------------------------------
 * Generic socket/networking constants etc.
 */
#if !defined(__WIN32__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/if.h>
#else			 /* __WIN32__ */
#if (__CYGWIN__)
#include <sys/socket.h>
#endif			/* __CYGWIN__ */
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <win32/in_systm.h>
#endif			 /* __WIN32__ */
#ifndef __sun__
#include <netinet/ether.h>
#include <net/ethernet.h>
#else			/* __sun__ */
#include <net/if.h>
#include <netinet/if_ether.h>
#endif			/* __sun__ */
#define	ARL_SECURE_PORT_1	33233
#define	ARL_SECURE_PORT_2	22
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN	46
#endif /* INET6_ADDRSTRLEN */
#ifndef HEADER_OFFSET
#define HEADER_OFFSET	"  "
#endif
/*--------------------------------------------------------------*/
/* Use "libnet" structure for consistancy */
#ifndef	ipv4_addr6
struct x_libnet_in6_addr
{
	union
	{
		uint8_t __u6_addr8[16];
		uint16_t __u6_addr16[8];
		uint32_t __u6_addr32[4];
	} __u6_addr;				/* 128-bit IP6 address */
};
/* Typedefs for "in_addr" structs */
typedef struct x_libnet_in6_addr in_addr6_t;
#define ipv4_addr6 __u6_addr.__u6_addr32[3]
#endif /* ipv4_addr6 */
typedef struct in_addr in_addr4_t;
#ifndef IP_ADDR_TYPE_IPV4
#define IP_ADDR_TYPE_IPV4	1
#define IP_ADDR_TYPE_IPV6	2
#endif
/*--------------------------------------------------------------*/
#if 0
/* netinet/in.h  IPv6 address */
struct in6_addr
{
    union
    {
	uint8_t	u6_addr8[16];
	uint16_t u6_addr16[8];
	uint32_t u6_addr32[4];
    } in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
};
extern const in_addr6_t in6addr_any;        /* :: */
extern const in_addr6_t in6addr_loopback;   /* ::1 */
#define IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46
#define INET_N_ADDRSTRLEN	40	/* Length of a string containing the numeric IP address */
#endif
/*--------------------------------------------------------------*/
#ifdef	SOLARIS
#define LIBNET_BIG_ENDIAN	1
#else
#define LIBNET_LIL_ENDIAN	1
#endif
/* Mainly for documentation */
#ifndef SOCKET
#define SOCKET	int
#endif
/*-------------------------------------------------------------
 * For xreferencing codes and descriptions.
 */
typedef struct 
{
	int code;
	char* string;
} int_str_t;
/*****************************************************************
 * This will update the "pkt_res" structure
 *  IPSEC header
 *  Internet Protocol Security Protocol
 *  Encapsulating Security Payload Header Static header size: 12 bytes
 *  Encapsulating Security Payload Footer Base header size: 2 bytes
 */
#ifndef IPSEC_ESP_HDR_SIZE
#define IPSEC_ESP_HDR_SIZE 12
#endif	/* IPSEC_ESP_HDR_SIZE */
/*****************************************************************
 * This will update the "pkt_res" structure
 *  IPSEC header
#define IPPROTO_AH 51	
 *  Authentication Header Static Size: 16 bytes
 */
#ifndef IPSEC_AH_HDR_SIZE
#define IPSEC_AH_HDR_SIZE 16
#endif	/* IPSEC_AH_HDR_SIZE */
#define ICMP_TRACEROUTE				30	/* Traceroute */
#define ICMP_CONVERSION_ERROR		31	/* Conversion error */
#define ICMP_DOMAIN_REQUEST			37	/* Domain Name request */
#define ICMP_DOMAIN_REPLY			38	/* Domain Name reply */
#define ICMP_SECURITY_FAILURE		40	/* Photuris Security Failure */
/*****************************************************************
 * Definitions for internet protocol version 4.
 * IP header as Per RFC 791, September, 1981. from 
 * /usr/include/netinet/ip.h with __USE_BSD NOT defined
 */
#ifndef IP_OFFMASK 
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
#endif
#ifndef UDP_HDR_SIZE
#define UDP_HDR_SIZE 8
#endif	/* UDP_HDR_SIZE */
#ifndef TCP_HDR_SIZE
#define TCP_HDR_SIZE 20
#endif	/* TCP_HDR_SIZE */
#ifndef GRE_HDR_SIZE
#define GRE_HDR_SIZE 12
#endif	/* GRE_HDR_SIZE */
/*****************************************************************
 *  IPv6 header
 *  Internet Protocol, version 6
 *  Static header size: 40 bytes
 */
#ifndef IPV6_HDR_SIZE
#define IPV6_HDR_SIZE 40
#endif	/* IPV6_HDR_SIZE */
/*****************************************************************
 *  IPv6 frag header
		case IPPROTO_FRAGMENT:	//(44) IPv6 fragmentation header
 *  Internet Protocol, version 6
 *  Static header size: 8 bytes
 */
#ifndef IPV6_FRAG_HDR_SIZE
#define IPV6_FRAG_HDR_SIZE 8
#endif	/* IPV6_FRAG_HDR_SIZE */
/*****************************************************************
 *  IPv6 routing header
		case IPPROTO_ROUTING: 	//(43) IPv6 routing header
 *  Internet Protocol, version 6
 *  Base header size: 4 bytes
 */
#ifndef IPV6_ROUTING_HDR_SIZE
#define IPV6_ROUTING_HDR_SIZE 4
#endif	/* IPV6_ROUTING_HDR_SIZE */
/*****************************************************************
 *  IPv6 destination options header
		case IPPROTO_DSTOPTS: 	//(60) IPv6 destination options
 *  Internet Protocol, version 6
 *  Base header size: 2 bytes
 */
#ifndef IPV6_DESTOPTS_HDR_SIZE
#define IPV6_DESTOPTS_HDR_SIZE 2
#endif	/* IPV6_DESTOPTS_HDR_SIZE */
/*****************************************************************
 *  IPv6 hop by hop options header
    	case HOPOPTS:			// (0) IPv6 Hop-by-Hop options
 *  Internet Protocol, version 6
 *  Base header size: 2 bytes
 */
#ifndef IPV6_HBHOPTS_HDR_SIZE
#define IPV6_HBHOPTS_HDR_SIZE 2
#endif	/* IPV6_HBHOPTS_HDR_SIZE */
/*****************************************************************
 *  IPv6 no next header
		case IPPROTO_NONE:    	//(59) IPv6 no next header
 *  Internet Protocol, version 6
 *  Base header size: 2 bytes
 */
#ifndef IPV6_NO_NXT_HDR_SIZE
#define IPV6_NO_NXT_HDR_SIZE 2
#endif	/* IPV6_NO_NXT_HDR_SIZE */
/*****************************************************************
 *  ICMP6 header
 *  Internet Control Message Protocol v6
 *  Base header size: 8 bytes
 */
#ifndef ICMPV6_HDR_SIZE
#define ICMPV6_HDR_SIZE 8
#endif	/* ICMPV6_HDR_SIZE */
#ifndef ICMP6_ECHO
#define ICMP6_ECHO          128
#endif
#ifndef ICMP6_ECHOREPLY
#define ICMP6_ECHOREPLY     129
#endif
#ifndef ICMP6_UNREACH
#define ICMP6_UNREACH       1
#endif
#ifndef ICMP6_PKTTOOBIG
#define ICMP6_PKTTOOBIG     2
#endif
#ifndef ICMP6_TIMXCEED
#define ICMP6_TIMXCEED      3
#endif
#ifndef ICMP6_PARAMPROB
#define ICMP6_PARAMPROB     4
#endif
#ifndef ICMPV4_HDR_SIZE
#define ICMPV4_HDR_SIZE 20
#endif	/* ICMPV4_HDR_SIZE */
/*****************************************************************
 * Fetch a string keyed to the protocol from ip header
 * /usr/include/netinet/in.h , in6.h
 * /usr/include/netinet/in.h 	solaris
 */
#if 0
#define	IPPROTO_IP		0			/* dummy for IP */
define	IPPROTO_ICMP	1			/* control message protocol */
define	IPPROTO_IGMP	2			/* group control protocol */
define	IPPROTO_GGP		3			/* gateway^2 (deprecated) */
define	IPPROTO_ENCAP	4			/* IP in IP encapsulation */
define	IPPROTO_TCP		6			/* tcp */
define	IPPROTO_EGP		8			/* exterior gateway protocol */
define	IPPROTO_PUP		12			/* pup */
define	IPPROTO_UDP		17			/* user datagram protocol */
define	IPPROTO_IDP		22			/* xns idp */
define	IPPROTO_IPV6	41			/* IPv6 encapsulated in IP */
define	IPPROTO_ROUTING	43			/* Routing header for IPv6 */
define	IPPROTO_FRAGMENT 44			/* Fragment header for IPv6 */
define	IPPROTO_RSVP	46			/* rsvp */
define	IPPROTO_ESP		50			/* IPsec Encap. Sec. Payload */
define	IPPROTO_AH		51			/* IPsec Authentication Hdr. */
define	IPPROTO_ICMPV6	58			/* ICMP for IPv6 */
define	IPPROTO_NONE	59			/* No next header for IPv6 */
define	IPPROTO_DSTOPTS	60			/* Destination options */
define	IPPROTO_PIM		103			/* PIM routing protocol */
define	IPPROTO_SCTP	132			/* Stream Control */
define	IPPROTO_RAW		255			/* raw IP packet */
define	IPPROTO_MAX		256
#endif
#ifndef IPPROTO_RAW
#define IPPROTO_RAW		255
#endif
#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP	4
#endif
#ifndef IPPROTO_TP
#define IPPROTO_TP	29
#endif
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF	89
#endif
#ifndef IPPROTO_MTP
#define IPPROTO_MTP	92
#endif
#ifndef IPPROTO_COMP
#define IPPROTO_COMP	108
#endif
#ifndef IPPROTO_L2TP
#define IPPROTO_L2TP	115
#endif
#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif
/* Solaris defines */
#ifndef	IPPROTO_HELLO
#define	IPPROTO_HELLO	63			/* "hello" routing protocol */
#endif
#ifndef	IPPROTO_ND
#define	IPPROTO_ND		77			/* UNOFFICIAL net disk proto */
#endif
#ifndef	IPPROTO_EON
#define	IPPROTO_EON		80			/* ISO clnp */
#endif
/*****************************************************************
 * Definitions for internet protocol version 4.
 * IP header as Per RFC 791, September, 1981. from 
 * /usr/include/netinet/ip.h with __USE_BSD NOT defined
 */
#ifndef IP_HDR_SIZE
#define IP_HDR_SIZE 20
#endif	/* IP_HDR_SIZE */
/*****************************************************************
 *  Address Resolution Protocol
 *  ARP header
 * /usr/include/net/if_arp.h(reference RFC 826)
 * /usr/include/netinet/if_ether.h(reference RFC 826)
 *  Base header size: 8 bytes
 */
#ifndef ARP_HDR_SIZE
#define ARP_HDR_SIZE 18
#endif	/* ARP_HDR_SIZE */
#ifndef ARPOP_NAK
#define ARPOP_NAK	10	/* (ATM)ARP NAK (linux defined it) */
#endif
/*****************************************************************
 * Points to beginning of packet.
 * 	1.	ethernet header
 *  2.	ip header(will have length of "ip data")
 *  3.		ip data
 *  4.		a. header(udp, tcp, icmp)
 *  5.		b  data(udp, tcp, icmp)
 * 
 * from /usr/include/net/ethernet.h 
 * 0                   1           
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Destination Address        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Destination Address        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Destination Address        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       Source Address          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       Source Address          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       Source Address          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        Frame type             | IP/IPv6/ARP/RARP
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                
 *           46 - 1500
 *           Data bytes
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                
 *
 *            Ethernet packet
 * *Preamble(used for synchronization), 8 bytes(64 bits)
 * Destination MAC Address, 6 bytes(48 bits)
 * Source MAC Address, 6 bytes(48 bits)
 * Type of Frame, 2 bytes(16 bits)
 * Data, 46-1500 bytes
 * *CRC(error detection), 4 bytes(32 bits)
 *
 * Note: Software packet goes from 64-1518 bytes(doesn't include
 * preamble or CRC).
 */
/*****************************************************************
 * IEEE 802.1Q (Virtual Local Area Network) VLAN header, static header 
 *  Ethernet VLAN header
 *  Static header size: 18 bytes
 */
#define ETHERNET_VLAN_HDR_SIZE 18
typedef struct
{
    uint8_t  ether_dhost[ETHER_ADDR_LEN];/* destination ethernet address */
    uint8_t  ether_shost[ETHER_ADDR_LEN];/* source ethernet address */
	uint16_t tpid:16;		/* Tag protocol ID (always 0x8100) */
	struct
	{
		uint16_t priority:3;	/* priority */
		uint16_t cfi:1;			/* Canonical format indicator */
		uint16_t vid:12;		/* VLAN identifier (0-4095) */
	} tag;
    uint16_t ether_type;                 /* protocol */
} vlan_hdr_t;
/*****************************************************************
 * /usr/include/netinet/ether.h 
 * Ethernet types.
 */
#ifndef ETHERTYPE_IEEE8023
#define	ETHERTYPE_IEEE8023		0x05FF	/* Hack for IEEE 802.3 packets */
#endif
#ifndef ETHERTYPE_PUP
#define	ETHERTYPE_PUP		0x0200	/* PUP protocol */
#endif
#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#endif
#ifndef ETHERTYPE_ARP
#define ETHERTYPE_ARP		0x0806	/* Addr. resolution protocol */
#endif
#ifndef ETHERTYPE_REVARP
#define ETHERTYPE_REVARP	0x8035	/* reverse Addr. resolution protocol */
#endif
#ifndef ETHERTYPE_NS
#define ETHERTYPE_NS		0x0600
#endif
#ifndef	ETHERTYPE_SPRITE
#define	ETHERTYPE_SPRITE	0x0500
#endif
#ifndef ETHERTYPE_TRAIL
#define ETHERTYPE_TRAIL		0x1000
#endif
#ifndef	ETHERTYPE_MOPDL
#define	ETHERTYPE_MOPDL		0x6001
#endif
#ifndef	ETHERTYPE_MOPRC
#define	ETHERTYPE_MOPRC		0x6002
#endif
#ifndef	ETHERTYPE_DN
#define	ETHERTYPE_DN		0x6003
#endif
#ifndef	ETHERTYPE_LAT
#define	ETHERTYPE_LAT		0x6004
#endif
#ifndef ETHERTYPE_SCA
#define ETHERTYPE_SCA		0x6007
#endif
#ifndef ETHERTYPE_REVARP
#define ETHERTYPE_REVARP	0x8035
#endif
#ifndef	ETHERTYPE_LANBRIDGE
#define	ETHERTYPE_LANBRIDGE	0x8038
#endif
#ifndef	ETHERTYPE_DECDNS
#define	ETHERTYPE_DECDNS	0x803c
#endif
#ifndef	ETHERTYPE_DECDTS
#define	ETHERTYPE_DECDTS	0x803e
#endif
#ifndef	ETHERTYPE_VEXP
#define	ETHERTYPE_VEXP		0x805b
#endif
#ifndef	ETHERTYPE_VPROD
#define	ETHERTYPE_VPROD		0x805c
#endif
#ifndef ETHERTYPE_ATALK
#define ETHERTYPE_ATALK		0x809b
#endif
#ifndef ETHERTYPE_AARP
#define ETHERTYPE_AARP		0x80f3
#endif
#ifndef	ETHERTYPE_8021Q
#define	ETHERTYPE_8021Q		0x8100
#endif
#ifndef ETHERTYPE_IPX
#define ETHERTYPE_IPX		0x8137
#endif
#ifndef ETHERTYPE_IPV6 
#define ETHERTYPE_IPV6		0x86DD		/* IPv6 RFC 2464*/
#endif
#ifndef ETHERTYPE_PPP
#define	ETHERTYPE_PPP		0x880b
#endif
#ifndef	ETHERTYPE_MPLS
#define	ETHERTYPE_MPLS		0x8847
#endif
#ifndef	ETHERTYPE_MPLS_MULTI
#define	ETHERTYPE_MPLS_MULTI	0x8848
#endif
#ifndef ETHERTYPE_PPPOED
#define ETHERTYPE_PPPOED	0x8863
#endif
#ifndef ETHERTYPE_PPPOES
#define ETHERTYPE_PPPOES	0x8864
#endif
#ifndef	ETHERTYPE_LOOPBACK
#define	ETHERTYPE_LOOPBACK	0x9000
#endif
#ifndef ETHERNET_HDR_SIZE
#define ETHERNET_HDR_SIZE 14
#endif	/* ETHERNET_HDR_SIZE */

#ifdef __cplusplus
}
#endif
#endif /* inet_defs_h */
/*------------------------EOF------------------------------*/
