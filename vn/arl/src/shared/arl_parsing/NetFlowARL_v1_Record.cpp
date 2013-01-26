/* 
 * File:   NetFlowARL_v1_Record.cpp
 * Author: Mike
 * 
 * Created on August 3, 2009, 10:56 PM
 */

#include <string.h>

#include "NetFlowARL_v1_Record.h"

namespace vn {
    namespace arl {
	namespace shared {

	    #define DATA ((flow_v1_record_t*)this->data)

	    typedef std::string::size_type size_type;

	    NetFlowARL_v1_Record::NetFlowARL_v1_Record():
	    NetFlowARLRecord(NetFlowARLRecord::NetFlow_v1) {
	    }

	    NetFlowARL_v1_Record::~NetFlowARL_v1_Record() {
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data
	    size_type NetFlowARL_v1_Record::size() const {
		return sizeof (flow_v1_record_t);
	    }

	    bool NetFlowARL_v1_Record::continue_serialize(char* dest) const {
		flow_v1_record_t* d = (flow_v1_record_t*)dest;

		HTONL(d->client_packets);
		HTONL(d->server_packets);
		HTONL(d->client_bytes);
		HTONL(d->server_bytes);
		HTONL(d->client_data_bytes);
		HTONL(d->server_data_bytes);
		HTONS(d->src_port);
		HTONS(d->dst_port);
		HTONS(d->window_size);

		return true;
	    }

	    bool NetFlowARL_v1_Record::continue_unserialize(char* data) {
		flow_v1_record_t* d = (flow_v1_record_t*)data;

		memset(&client_ip, 0, sizeof(in_addr6_t));
		memset(&server_ip, 0, sizeof(in_addr6_t));

		client_ip.__u6_addr.__u6_addr32[3] = d->src_ip;
		server_ip.__u6_addr.__u6_addr32[3] = d->dst_ip;

		NTOHL(d->client_packets);
		NTOHL(d->server_packets);
		NTOHL(d->client_bytes);
		NTOHL(d->server_bytes);
		NTOHL(d->client_data_bytes);
		NTOHL(d->server_data_bytes);
		NTOHS(d->src_port);
		NTOHS(d->dst_port);
		NTOHS(d->window_size);

		return true;
	    }

	    /// \returns Client IP
	    const in_addr6_t* NetFlowARL_v1_Record::getClientIPv6() const {
		return &(this->client_ip);
	    }

	    /// \returns Server IP
	    const in_addr6_t* NetFlowARL_v1_Record::getServerIPv6() const {
		return &(this->server_ip);
	    }

	    /// \returns Packets sent by client
	    const uint32_t NetFlowARL_v1_Record::getClientPacketCount() const {
		return DATA->client_packets;
	    }

	    /// \returns Packets sent by server
	    const uint32_t NetFlowARL_v1_Record::getServerPacketCount() const {
		return DATA->server_packets;
	    }

	    /// \returns Bytes sent by client
	    const uint32_t NetFlowARL_v1_Record::getClientByteCount() const {
		return DATA->client_bytes;
	    }

	    /// \returns Bytes sent by server
	    const uint32_t NetFlowARL_v1_Record::getServerByteCount() const {
		return DATA->server_bytes;
	    }

	    /// \returns Data bytes (layer 7) sent by client
	    const uint32_t NetFlowARL_v1_Record::getClientDataCount() const {
		return DATA->client_data_bytes;
	    }

	    /// \returns Data bytes (Layer 7) sent by server
	    const uint32_t NetFlowARL_v1_Record::getServerDataCount() const {
		return DATA->server_data_bytes;
	    }

	    /// \returns Client port
	    const uint16_t NetFlowARL_v1_Record::getClientPort() const {
		return DATA->src_port;
	    }

	    /// \returns Server port
	    const uint16_t NetFlowARL_v1_Record::getServerPort() const {
		return DATA->dst_port;
	    }

	    /// \returns Client Window size if TCP
	    const uint16_t NetFlowARL_v1_Record::getClientWindowSize() const {
		return DATA->window_size;
	    }

	    /// \returns Server Window size if TCP
	    const uint16_t NetFlowARL_v1_Record::getServerWindowSize() const {
		return DATA->window_size;
	    }

	    /// \returns IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
	    const uint8_t NetFlowARL_v1_Record::getProtocol() const {
		return DATA->protocol;
	    }

	    /// \returns Or if all TCP flags (0 for non-TCP)
	    const uint8_t NetFlowARL_v1_Record::getTcpFlags() const {
		return DATA->tcp_flags;
	    }

	    /// \returns Flag if window size changed
	    const uint8_t NetFlowARL_v1_Record::getWindowChanged() const {
		return DATA->window_changed;
	    }

	    /// \returns Time to live
	    const uint8_t NetFlowARL_v1_Record::getTtl() const {
		return DATA->ttl;
	    }

	    /// \returns 1 if session considered closed
	    const uint8_t NetFlowARL_v1_Record::getSessionClosed() const {
		return DATA->session_closed;
	    }
	} // namespace shared
    } // namespace arl
} // namespace vn
