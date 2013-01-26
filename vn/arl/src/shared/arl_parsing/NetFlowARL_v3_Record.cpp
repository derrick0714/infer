/* 
 * File:   NetFlowARL_v3_Record.cpp
 * Author: Mike
 * 
 * Created on August 4, 2009, 9:14 PM
 */

#include "NetFlowARL_v3_Record.h"

namespace vn {
    namespace arl {
        namespace shared {
	    
	    #define DATA ((flow_v3_record_t*)this->data)

	    typedef std::string::size_type size_type;

	    NetFlowARL_v3_Record::NetFlowARL_v3_Record():
	    NetFlowARLRecord(NetFlowARLRecord::NetFlow_v3){
	    }

	    NetFlowARL_v3_Record::~NetFlowARL_v3_Record() {
	    }

	    bool NetFlowARL_v3_Record::continue_serialize(char* dest) const {
		flow_v3_record_t* d = (flow_v3_record_t*)dest;

		HTONL(d->c_packets);
		HTONL(d->s_packets);
		HTONL(d->c_bytes);
		HTONL(d->s_bytes);
		HTONL(d->c_data_bytes);
		HTONL(d->s_data_bytes);
		HTONL(d->c_time);
		HTONL(d->s_time);
		HTONL(d->tuple_hash);
		HTONL(d->flow_id.c_time.i);
		HTONL(d->flow_id.s_time.i);
		HTONS(d->c_port);
		HTONS(d->s_port);
		HTONS(d->c_ether_type);
		HTONS(d->s_ether_type);
		HTONS(d->s_window_size);
		HTONS(d->s_window_size);

		return true;
	    }

	    bool NetFlowARL_v3_Record::continue_unserialize(char* data) {
		flow_v3_record_t* d = (flow_v3_record_t*)data;

		if(d->check_char != FLOW_V3_CHECK_CHAR) {
		    return false;
		}

		NTOHL(d->c_packets);
		NTOHL(d->s_packets);
		NTOHL(d->c_bytes);
		NTOHL(d->s_bytes);
		NTOHL(d->c_data_bytes);
		NTOHL(d->s_data_bytes);
		NTOHL(d->c_time);
		NTOHL(d->s_time);
		NTOHL(d->tuple_hash);
		NTOHL(d->flow_id.c_time.i);
		NTOHL(d->flow_id.s_time.i);
		NTOHS(d->c_port);
		NTOHS(d->s_port);
		NTOHS(d->c_ether_type);
		NTOHS(d->s_ether_type);
		NTOHS(d->s_window_size);
		NTOHS(d->s_window_size);

		return true;
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data
	    size_type NetFlowARL_v3_Record::size() const {
		return sizeof (flow_v3_record_t);
	    }

	    /// \returns Client IP
	    const in_addr6_t* NetFlowARL_v3_Record::getClientIPv6() const {
		return &(DATA->c_ip);
	    }

	    /// \returns Server IP
	    const in_addr6_t* NetFlowARL_v3_Record::getServerIPv6() const {
		return &(DATA->s_ip);
	    }

	    /// \returns Packets sent by client
	    const uint32_t NetFlowARL_v3_Record::getClientPacketCount() const {
		return DATA->c_packets;
	    }

	    /// \returns Packets sent by server
	    const uint32_t NetFlowARL_v3_Record::getServerPacketCount() const {
		return DATA->s_packets;
	    }

	    /// \returns Bytes sent by client
	    const uint32_t NetFlowARL_v3_Record::getClientByteCount() const {
		return DATA->c_bytes;
	    }

	    /// \returns Bytes sent by server
	    const uint32_t NetFlowARL_v3_Record::getServerByteCount() const {
		return DATA->s_bytes;
	    }

	    /// \returns Data bytes (layer 7) sent by client
	    const uint32_t NetFlowARL_v3_Record::getClientDataCount() const {
		return DATA->c_data_bytes;
	    }

	    /// \returns Data bytes (Layer 7) sent by server
	    const uint32_t NetFlowARL_v3_Record::getServerDataCount() const {
		return DATA->s_data_bytes;
	    }

	    /// \returns Client side "flow index" (condensed timestamp)
	    const uint32_t NetFlowARL_v3_Record::getClientPDXid() const {
		return DATA->flow_id.c_time.i;
	    }

	    /// \returns Server side "flow index" (condensed timestamp)
	    const uint32_t NetFlowARL_v3_Record::getServerPDXid() const {
		return DATA->flow_id.s_time.i;
	    }

	    /// \returns hashed 3 tuple (ip's and server port)
	    const uint32_t NetFlowARL_v3_Record::getTupleHash() const {
		return DATA->tuple_hash;
	    }

	    /// \returns Flags dealing with the bidirectional session
	    const session_flags_t* NetFlowARL_v3_Record::getSessionFlags() const {
		return &(DATA->session_flags);
	    }

	    /// \returns Client flow flags
	    const flow_flags_t* NetFlowARL_v3_Record::getClientFlowFlags() const {
		return &(DATA->c_flow_flags);
	    }

	    /// \returns Server flow flags
	    const flow_flags_t* NetFlowARL_v3_Record::getServerFlowFlags() const {
		return &(DATA->s_flow_flags);
	    }

	    /// \returns Client port
	    const uint16_t NetFlowARL_v3_Record::getClientPort() const {
		return DATA->c_port;
	    }

	    /// \returns Server port
	    const uint16_t NetFlowARL_v3_Record::getServerPort() const {
		return DATA->c_port;
	    }

	    /// \returns Client ethernet packet (layer 3) type
	    const uint16_t NetFlowARL_v3_Record::getClientEthernetType() const {
		return DATA->c_ether_type;
	    }

	    /// \returns Server ethernet packet (layer 3) type
	    const uint16_t NetFlowARL_v3_Record::getServerEthernetType() const {
		return DATA->s_ether_type;
	    }

	    /// \returns Client Window size if TCP
	    const uint16_t NetFlowARL_v3_Record::getClientWindowSize() const {
		return DATA->c_window_size;
	    }

	    /// \returns Server Window size if TCP
	    const uint16_t NetFlowARL_v3_Record::getServerWindowSize() const {
		return DATA->s_window_size;
	    }

	    /// \returns IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
	    const uint8_t NetFlowARL_v3_Record::getProtocol() const {
		return DATA->session_flags.ip_proto_2;
	    }

	    /// \returns Or if all TCP flags (0 for non-TCP)
	    const uint8_t NetFlowARL_v3_Record::getTcpFlags() const {
		return DATA->session_flags.flags;
	    }

	    /// \returns Time to live
	    const uint8_t NetFlowARL_v3_Record::getTtl() const {
		return DATA->c_flow_flags.ttl;
	    }
        } // namespace shared
    } // namespace arl
} // namespace vn