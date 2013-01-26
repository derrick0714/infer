/* 
 * File:   ProtocolNumbers.h
 * Author: Mike
 *
 * Created on January 9, 2010, 10:01 PM
 *
 * This file provides the facilities to map IP protocol number
 * to/from string. I didn't find a good way to map to string
 * so here it is.
 *
 * source: http://www.iana.org/assignments/protocol-numbers/
 */

#ifndef _PROTOCOLNUMBERS_H
#define	_PROTOCOLNUMBERS_H

#include <string>

/**
 *  \brief returns a string containing the protocol name as
 *         defined by iana (http://www.iana.org/assignments/protocol-numbers/)
 */
std::string getProtocolString(const int num) {
    // Note the function is done this way on purpose. To create
    // the modifiable string object on demand and to avoid a global.
    // The compiler will optimize to a simple jump table.

    using namespace std;

    switch(num) {
        case 0: return string("HOPOPT");
        case 1: return string("ICMP");
        case 2: return string("IGMP");
        case 3: return string("GGP");
        case 4: return string("IP");
        case 5: return string("ST");
        case 6: return string("TCP");
        case 7: return string("CBT");
        case 8: return string("EGP");
        case 9: return string("IGP");
        case 10: return string("BBN-RCC-MON");
        case 11: return string("NVP-II");
        case 12: return string("PUP");
        case 13: return string("ARGUS");
        case 14: return string("EMCON");
        case 15: return string("XNET");
        case 16: return string("CHAOS");
        case 17: return string("UDP");
        case 18: return string("MUX");
        case 19: return string("DCN-MEAS");
        case 20: return string("HMP");
        case 21: return string("PRM");
        case 22: return string("XNS-IDP");
        case 23: return string("TRUNK-1");
        case 24: return string("TRUNK-2");
        case 25: return string("LEAF-1");
        case 26: return string("LEAF-2");
        case 27: return string("RDP");
        case 28: return string("IRTP");
        case 29: return string("ISO-TP4");
        case 30: return string("NETBLT");
        case 31: return string("MFE-NSP");
        case 32: return string("MERIT-INP");
        case 33: return string("DCCP");
        case 34: return string("3PC");
        case 35: return string("IDPR");
        case 36: return string("XTP");
        case 37: return string("DDP");
        case 38: return string("IDPR-CMTP");
        case 39: return string("TP++");
        case 40: return string("IL");
        case 41: return string("IPv6");
        case 42: return string("SDRP");
        case 43: return string("IPv6-Route");
        case 44: return string("IPv6-Frag");
        case 45: return string("IDRP");
        case 46: return string("RSVP");
        case 47: return string("GRE");
        case 48: return string("DSR");
        case 49: return string("BNA");
        case 50: return string("ESP");
        case 51: return string("AH");
        case 52: return string("I-NLSP");
        case 53: return string("SWIPE");
        case 54: return string("NARP");
        case 55: return string("MOBILE");
        case 56: return string("TLSP");
        case 57: return string("SKIP");
        case 58: return string("IPv6-ICMP");
        case 59: return string("IPv6-NoNxt");
        case 60: return string("IPv6-Opts");
        case 61: return string("any");
        case 62: return string("CFTP");
        case 63: return string("any");
        case 64: return string("SAT-EXPAK");
        case 65: return string("KRYPTOLAN");
        case 66: return string("RVD");
        case 67: return string("IPPC");
        case 68: return string("distributed file system");
        case 69: return string("SAT-MON");
        case 70: return string("VISA");
        case 71: return string("IPCV");
        case 72: return string("CPNX");
        case 73: return string("CPHB");
        case 74: return string("WSN");
        case 75: return string("PVP");
        case 76: return string("BR-SAT-MON");
        case 77: return string("SUN-ND");
        case 78: return string("WB-MON");
        case 79: return string("WB-EXPAK");
        case 80: return string("ISO-IP");
        case 81: return string("VMTP");
        case 82: return string("SECURE-VMTP");
        case 83: return string("VINES");
        case 84: return string("TTP");
        case 85: return string("NSFNET-IGP");
        case 86: return string("DGP");
        case 87: return string("TCF");
        case 88: return string("EIGRP");
        case 89: return string("OSPFIGP");
        case 90: return string("Sprite-RPC");
        case 91: return string("LARP");
        case 92: return string("MTP");
        case 93: return string("AX.25");
        case 94: return string("IPIP");
        case 95: return string("MICP");
        case 96: return string("SCC-SP");
        case 97: return string("ETHERIP");
        case 98: return string("ENCAP");
        case 99: return string("Private Encryption");
        case 100: return string("GMTP");
        case 101: return string("IFMP");
        case 102: return string("PNNI");
        case 103: return string("PIM");
        case 104: return string("ARIS");
        case 105: return string("SCPS");
        case 106: return string("QNX");
        case 107: return string("A/N");
        case 108: return string("IPComp");
        case 109: return string("SNP");
        case 110: return string("Compaq-Peer");
        case 111: return string("IPX-in-IP");
        case 112: return string("VRRP");
        case 113: return string("PGM");
        case 114: return string("any");
        case 115: return string("L2TP");
        case 116: return string("DDX");
        case 117: return string("IATP");
        case 118: return string("STP");
        case 119: return string("SRP");
        case 120: return string("UTI");
        case 121: return string("SMP");
        case 122: return string("SM");
        case 123: return string("PTP");
        case 124: return string("ISIS");
        case 125: return string("FIRE");
        case 126: return string("CRTP");
        case 127: return string("CRUDP");
        case 128: return string("SSCOPMCE");
        case 129: return string("IPLT");
        case 130: return string("SPS");
        case 131: return string("PIPE");
        case 132: return string("SCTP");
        case 133: return string("FC");
        case 134: return string("RSVP-E2E-IGNORE");
        case 135: return string("Mobility");
        case 136: return string("UDPLite");
        case 137: return string("MPLS-in-IP");
        case 138: return string("manet");
        case 139: return string("HIP");
        case 140: return string("Shim6");
        case 253: return string("Testing");
        case 254: return string("Testing");
        case 255: return string("Reserved");
    }

    if(num < 0 || num > 255) {
        return string("Error");
    }

    return string("Unassigned");
}

#endif	/* _PROTOCOLNUMBERS_H */

