#include <stdio.h>
#include <arpa/inet.h>
#include <arl/arl_types.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "PDXRecord.h"

namespace vn {
    namespace arl {
	namespace shared {

	    typedef std::string::size_type size_type;

	    using namespace boost::posix_time;
	    using namespace std;

	    PDXRecord::PDXRecord() {
	    }

	    PDXRecord::PDXRecord(TimeStamp* b_time) {
		base_time = *b_time;
	    }

	    /// \brief Virtual destructor

	    PDXRecord::~PDXRecord() {
	    }

	    /// \brief Get the start time of the data
	    /// \returns the start time of the data

	    TimeStamp PDXRecord::startTime() const {
		return packet_timestamp;
	    }

	    /// \brief Get the end time of the data
	    /// \returns the end time of the data

	    TimeStamp PDXRecord::endTime() const {
		return packet_timestamp;
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data

	    size_type PDXRecord::size() const {
		return sizeof (pidx_data_t);
	    }

	    /// \brief Serialize data
	    /// \param ostr the string in which to store the serialized data
	    /// \returns true if the data was successfully serialized into ostr

	    bool PDXRecord::serialize(std::string &ostr) const {
		return false;
	    }

	    /// \brief Unserialize data
	    /// \param istr the string from which unserialize data
	    /// \returns true if the data was successfully unserialized from ostr

	    bool PDXRecord::unserialize(const std::string &istr) {
		pidx_data_t* arlRecord = (pidx_data_t*) istr.data();

		this->packet_timestamp = base_time;
		this->flow_timestamp = base_time;

		arlRecord->p_time.i = ntohl(arlRecord->p_time.i);
		arlRecord->f_time.i = ntohl(arlRecord->f_time.i);

		packet_timestamp += TimeStamp((arlRecord->p_time.i >> 20) & 0xFFF, arlRecord->p_time.i & 0xFFFFF);
		flow_timestamp += TimeStamp((arlRecord->f_time.i >> 20) & 0xFFF, arlRecord->f_time.i & 0xFFFFF);

		arlRecord->flags.i = htonl(arlRecord->flags.i);
		flags = arlRecord->flags;
		hash = ntohl(arlRecord->tuple_hash);
		port = ntohs(arlRecord->port);
		fld_1 = arlRecord->fld_1;
		fld_2 = arlRecord->fld_2;

		return true;
	    }

	    const TimeStamp& PDXRecord::getBaseTime() const {
		return base_time;
	    }

	    const TimeStamp& PDXRecord::getPacketTime() const {
		return packet_timestamp;
	    }

	    const TimeStamp& PDXRecord::getFlowTime() const {
		return flow_timestamp;
	    }

	    const pidx_data_flags_t& PDXRecord::getPacketFlags() const {
		return flags;
	    }

	    const uint32_t& PDXRecord::getHash() const {
		return hash;
	    }

	    const uint16_t& PDXRecord::getPort() const {
		return port;
	    }

	    const uint8_t& PDXRecord::getField1() const {
		return fld_1;
	    }

	    const uint8_t& PDXRecord::getField2() const {
		return fld_2;
	    }

	    const std::string PDXRecord::toString() const {
		/* Example:
		    IP_F=0x0
		      Fld_1=0x00 Fld_2=0x10 Hash=F6C7171D Flow_Ts=00:01.198144(00130600)
			 2-- 00:01.198158(0013060E) p36870 Clnt Cnt=12(0x000C) Frag=0 TTL=0x39
		 */
		char buffer[2048];

		string ft = to_simple_string((ptime) flow_timestamp);
		string pt = to_simple_string((ptime) packet_timestamp);

		snprintf(buffer, 2047, "IP_F=0x%X\n  Fld_1=0x%X Fld_2=0x%X Hash=%08X Flow_Ts=%s\n  2--%s p%d Clnt Cnt=%d(%04X) Frag=%d TTL=0x%02X\n",
			0, fld_1, fld_2, hash, ft.c_str(), pt.c_str(), port, flags.f.data_len, flags.f.data_len,
			flags.f.frag, flags.f.ttl);

		return string(buffer);
	    }


	} // namespace shared
    } // namespace arl
} // namespace vn
