#include "../../debug/dbg.h"

#include <string>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "NetFlowARLRecord.h"

namespace vn {
    namespace arl {
	namespace shared {

	    typedef std::string::size_type size_type;

	    using namespace std;

	    NetFlowARLRecord::NetFlowARLRecord(NetFlowVersion ver) : version(ver) {
	    }

	    /// \brief Virtual destructor

	    NetFlowARLRecord::~NetFlowARLRecord() {
	    }

	    /// \brief Get the start time of the data
	    /// \returns the start time of the data

	    TimeStamp NetFlowARLRecord::startTime() const {
		return start_time;
	    }

	    /// \brief Get the end time of the data
	    /// \returns the end time of the data

	    TimeStamp NetFlowARLRecord::endTime() const {
		return last_time;
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data

	    size_type NetFlowARLRecord::size() const {
		return 0;
	    }

	    /// \brief Serialize data
	    /// \param ostr the string in which to store the serialized data
	    /// \returns true if the data was successfully serialized into ostr

	    bool NetFlowARLRecord::serialize(std::string &ostr) const {
		char sdata[sizeof(data)];

		memcpy(sdata, data, sizeof(data));

		// revert back the ordering changes.
		flow_time_sect* time = (flow_time_sect*)sdata;
		HTONL(time->first_secs);
		HTONL(time->first_usecs);
		HTONL(time->last_secs);
		HTONL(time->last_usecs);

		if(!continue_serialize(sdata)) {
		    return false;
		}

		ostr.clear();
		ostr.insert(0, sdata, sizeof(sdata));

		return true;
	    }

	    bool NetFlowARLRecord::continue_serialize(char* dest) const {
		return false;
	    }

	    /// \brief Unserialize data
	    /// \param istr the string from which unserialize data
	    /// \returns true if the data was successfully unserialized from ostr

	    bool NetFlowARLRecord::unserialize(const std::string &istr) {
		const char* strData = istr.data();
		int cpySize = istr.length();

		if(cpySize > sizeof(data)) {
		    // Should never happen but we don't want to read/write past the boundary
		    cpySize = sizeof(data);
		}

		memcpy(data, strData, cpySize);

		flow_time_sect* time = (flow_time_sect*)data;
		
		NTOHL(time->first_secs);
		NTOHL(time->first_usecs);
		NTOHL(time->last_secs);
		NTOHL(time->last_usecs);

		start_time.set(time->first_secs, time->first_usecs);
		last_time.set(time->last_secs, time->last_usecs);

		return continue_unserialize(data);
	    }

	    const NetFlowARLRecord::NetFlowVersion NetFlowARLRecord::getVersion() const {
		return version;
	    }

	    bool NetFlowARLRecord::continue_unserialize(char* data) {
		return true;
	    }

	    const boost::asio::ip::address NetFlowARLRecord::getClientIP() const {
		using namespace boost::asio::ip;
		
		const in_addr6_t* addr = this->getClientIPv6();

                if(addr->__u6_addr.__u6_addr32[0] |
                   addr->__u6_addr.__u6_addr32[1] |
                   addr->__u6_addr.__u6_addr32[2] != 0) {
                    // we are dealing with ipv6
                    boost::array<unsigned char, sizeof(in_addr6_t)> addBytes;

                    memcpy(addBytes.elems, (unsigned char*)addr, sizeof(in_addr6_t));

                    return address(address_v6(addBytes));
                } else {
                    boost::array<unsigned char, 4> addBytes;

                    memcpy(addBytes.elems, ((unsigned char*)addr) + sizeof(in_addr6_t) - 4, sizeof(in_addr6_t));

                    return address(address_v4(addBytes));
                }
	    }
                
	    const boost::asio::ip::address NetFlowARLRecord::getServerIP() const {
		using namespace boost::asio::ip;

		const in_addr6_t* addr = this->getServerIPv6();
		if(addr->__u6_addr.__u6_addr32[0] |
                   addr->__u6_addr.__u6_addr32[1] |
                   addr->__u6_addr.__u6_addr32[2] != 0) {
                    // we are dealing with ipv6
                    boost::array<unsigned char, sizeof(in_addr6_t)> addBytes;

                    memcpy(addBytes.elems, (unsigned char*)addr, sizeof(in_addr6_t));

                    return address(address_v6(addBytes));
                } else {
                    boost::array<unsigned char, 4> addBytes;

                    memcpy(addBytes.elems, ((unsigned char*)addr) + sizeof(in_addr6_t) - 4, sizeof(in_addr6_t));

                    return address(address_v4(addBytes));
                }
	    }

	    /// \returns Client IP
	    const in_addr6_t* NetFlowARLRecord::getClientIPv6() const {
		return NULL;
	    }

	    /// \returns Server IP
	    const in_addr6_t* NetFlowARLRecord::getServerIPv6() const {
                cout << "I shouldn't be here" << endl;
		return NULL;
	    }

	    /// \returns Packets sent by client
	    const uint32_t NetFlowARLRecord::getClientPacketCount() const {
		return 0;
	    }

	    /// \returns Packets sent by server
	    const uint32_t NetFlowARLRecord::getServerPacketCount() const {
		return 0;
	    }

	    /// \returns Bytes sent by client
	    const uint32_t NetFlowARLRecord::getClientByteCount() const {
		return 0;
	    }

	    /// \returns Bytes sent by server
	    const uint32_t NetFlowARLRecord::getServerByteCount() const {
		return 0;
	    }

	    /// \returns Data bytes (layer 7) sent by client
	    const uint32_t NetFlowARLRecord::getClientDataCount() const {
		return 0;
	    }

	    /// \returns Data bytes (Layer 7) sent by server
	    const uint32_t NetFlowARLRecord::getServerDataCount() const {
		return 0;
	    }

	    /// \returns Client side "flow index" (condensed timestamp)
	    const uint32_t NetFlowARLRecord::getClientPDXid() const {
		return 0;
	    }

	    /// \returns Server side "flow index" (condensed timestamp)
	    const uint32_t NetFlowARLRecord::getServerPDXid() const {
		return 0;
	    }

	    /// \returns hashed 3 tuple (ip's and server port)
	    const uint32_t NetFlowARLRecord::getTupleHash() const {
		return 0;
	    }

	    /// \returns Flags dealing with the bidirectional session
	    const session_flags_t* NetFlowARLRecord::getSessionFlags() const {
		return NULL;
	    }

	    /// \returns Client flow flags
	    const flow_flags_t* NetFlowARLRecord::getClientFlowFlags() const {
		return NULL;
	    }

	    /// \returns Server flow flags
	    const flow_flags_t* NetFlowARLRecord::getServerFlowFlags() const {
		return NULL;
	    }
	    
	    /// \returns Client port
	    const uint16_t NetFlowARLRecord::getClientPort() const {
		return 0;
	    }

	    /// \returns Server port
	    const uint16_t NetFlowARLRecord::getServerPort() const {
		return 0;
	    }

	    /// \returns Client ethernet packet (layer 3) type
	    const uint16_t NetFlowARLRecord::getClientEthernetType() const {
		return 0;
	    }

	    /// \returns Server ethernet packet (layer 3) type
	    const uint16_t NetFlowARLRecord::getServerEthernetType() const {
		return 0;
	    }

	    /// \returns Client Window size if TCP
	    const uint16_t NetFlowARLRecord::getClientWindowSize() const {
		return 0;
	    }

	    /// \returns Server Window size if TCP
	    const uint16_t NetFlowARLRecord::getServerWindowSize() const {
		return 0;
	    }

	    /// \returns IP Protocol (ICMP = 1, TCP = 6, UDP = 17, ...)
	    const uint8_t NetFlowARLRecord::getProtocol() const {
		return 0;
	    }

	    /// \returns Or if all TCP flags (0 for non-TCP)
	    const uint8_t NetFlowARLRecord::getTcpFlags() const {
		return 0;
	    }

	    /// \returns Flag if window size changed
	    const uint8_t NetFlowARLRecord::getWindowChanged() const {
		return 0;
	    }

	    /// \returns Time to live
	    const uint8_t NetFlowARLRecord::getTtl() const {
		return 0;
	    }

	    /// \returns 1 if session considered closed
	    const uint8_t NetFlowARLRecord::getSessionClosed() const {
		return 0;
	    }
	} // namespace shared
    } // namespace arl
} // namespace vn
