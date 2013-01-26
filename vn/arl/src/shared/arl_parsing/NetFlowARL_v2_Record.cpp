/* 
 * File:   NetFlowARL_v2_Record.cpp
 * Author: Mike
 * 
 * Created on August 4, 2009, 9:11 PM
 */

#include "NetFlowARL_v2_Record.h"

namespace vn {
    namespace arl {
	namespace shared {

	    #define DATA ((flow_v2_record_t*)this->data)

	    typedef std::string::size_type size_type;

	    NetFlowARL_v2_Record::NetFlowARL_v2_Record():
	    NetFlowARLRecord(NetFlowARLRecord::NetFlow_v2){
	    }

	    NetFlowARL_v2_Record::~NetFlowARL_v2_Record() {
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data
	    size_type NetFlowARL_v2_Record::size() const {
		return sizeof (flow_v2_record_t);
	    }

	    bool NetFlowARL_v2_Record::continue_serialize(char* dest) const {
		flow_v2_record_t* d = (flow_v2_record_t*)dest;

		HTONL(d->c_packets);
		HTONL(d->s_packets);
		HTONL(d->c_bytes);
		HTONL(d->s_bytes);
		HTONL(d->c_data_bytes);
		HTONL(d->s_data_bytes);
		HTONS(d->c_port);
		HTONS(d->s_port);
		HTONS(d->c_ether_type);
		HTONS(d->s_ether_type);
		HTONS(d->s_window_size);
		HTONS(d->s_window_size);

		return true;
	    }

	    bool NetFlowARL_v2_Record::continue_unserialize(char* data) {
		flow_v2_record_t* d = (flow_v2_record_t*)data;

		if(d->check_char != FLOW_V2_CHECK_CHAR) {
		    return false;
		}

		NTOHL(d->c_packets);
		NTOHL(d->s_packets);
		NTOHL(d->c_bytes);
		NTOHL(d->s_bytes);
		NTOHL(d->c_data_bytes);
		NTOHL(d->s_data_bytes);
		NTOHS(d->c_port);
		NTOHS(d->s_port);
		NTOHS(d->c_ether_type);
		NTOHS(d->s_ether_type);
		NTOHS(d->s_window_size);
		NTOHS(d->s_window_size);

		return true;
	    }

	    /// \returns Client IP
	    const in_addr6_t* NetFlowARL_v2_Record::getClientIPv6() const {
		return &(DATA->c_ip);
	    }

	    /// \returns Server IP
	    const in_addr6_t* NetFlowARL_v2_Record::getServerIPv6() const {
		return &(DATA->s_ip);
	    }

	    /// \returns Packets sent by client
	    const uint32_t NetFlowARL_v2_Record::getClientPacketCount() const {
		return DATA->c_packets;
	    }

	    /// \returns Packets sent by server
	    const uint32_t NetFlowARL_v2_Record::getServerPacketCount() const {
		return DATA->s_packets;
	    }

	    /// \returns Bytes sent by client
	    const uint32_t NetFlowARL_v2_Record::getClientByteCount() const {
		return DATA->c_bytes;
	    }

	    /// \returns Bytes sent by server
	    const uint32_t NetFlowARL_v2_Record::getServerByteCount() const {
		return DATA->s_bytes;
	    }

	    /// \returns Data bytes (layer 7) sent by client
	    const uint32_t NetFlowARL_v2_Record::getClientDataCount() const {
		return DATA->c_data_bytes;
	    }

	    /// \returns Data bytes (Layer 7) sent by server
	    const uint32_t NetFlowARL_v2_Record::getServerDataCount() const {
		return DATA->s_data_bytes;
	    }

	    /// \returns Flags dealing with the bidirectional session
	    const session_flags_t* NetFlowARL_v2_Record::getSessionFlags() const {
		return &(DATA->session_flags);
	    }

	    /// \returns Client flow flags
	    const flow_flags_t* NetFlowARL_v2_Record::getClientFlowFlags() const {
		return &(DATA->c_flow_flags);
	    }

	    /// \returns Server flow flags
	    const flow_flags_t* NetFlowARL_v2_Record::getServerFlowFlags() const {
		return &(DATA->s_flow_flags);
	    }

	    /// \returns Client port
	    const uint16_t NetFlowARL_v2_Record::getClientPort() const {
		return DATA->c_port;
	    }

	    /// \returns Server port
	    const uint16_t NetFlowARL_v2_Record::getServerPort() const {
		return DATA->s_port;
	    }

	    /// \returns Client ethernet packet (layer 3) type
	    const uint16_t NetFlowARL_v2_Record::getClientEthernetType() const {
		return DATA->c_ether_type;
	    }

	    /// \returns Server ethernet packet (layer 3) type
	    const uint16_t NetFlowARL_v2_Record::getServerEthernetType() const {
		return DATA->s_ether_type;
	    }

	    /// \returns Client Window size if TCP
	    const uint16_t NetFlowARL_v2_Record::getClientWindowSize() const {
		return DATA->c_window_size;
	    }

	    /// \returns Server Window size if TCP
	    const uint16_t NetFlowARL_v2_Record::getServerWindowSize() const {
		return DATA->s_window_size;
	    }

	    /// \returns IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
	    const uint8_t NetFlowARL_v2_Record::getProtocol() const {
		return DATA->session_flags.ip_proto_2;
	    }

	    /// \returns Or if all TCP flags (0 for non-TCP)
	    const uint8_t NetFlowARL_v2_Record::getTcpFlags() const {
		return DATA->session_flags.flags;
	    }

	    /// \returns Time to live
	    const uint8_t NetFlowARL_v2_Record::getTtl() const {
		return DATA->c_flow_flags.ttl;
	    }
	} // namespace shared
    } // namespace arl
} // namespace vn