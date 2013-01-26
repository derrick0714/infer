#ifndef PDXRECORD_HPP
#define PDXRECORD_HPP

#include <string>

#include <arl/arl_types.h>

#include "../Serializable.hpp"

namespace vn {
namespace arl {
namespace shared {

    /// \brief Each record represents one packet.
    /// 
    /// Struct:
    ///     typedef union {
    ///      struct {
    ///         uint32_t client : 1; /* source field had largest port */
    ///         uint32_t frag : 1; /* fragmented packet */
    ///         uint32_t rsrvd : 3; /* Unused at this time */
    ///         uint32_t flags : 3; /* IP:flags, (D=don't frag, M=More frag) */
    ///         uint32_t ttl : 8; /* IP:time to live */
    ///         uint32_t data_len : 16; /* Number of data bytes */
    ///      } f;
    /// 
    ///      uint32_t i;
    ///     } pidx_data_flags_t; /* (4) "offset" timestamp */
    /// 
    ///
    /// typedef struct {
    ///     con_ts_t p_time;
    ///     con_ts_t f_time;
    ///     uint32_t tuple_hash; /* (4) 5 tuple hash */
    ///     pidx_data_flags_t flags; /* (4) data flags */
    ///     uint16_t port; /* (2) Ephemeral port (higher number) */
    ///     uint8_t fld_1; /* (1) ICMP:type */
    ///     uint8_t fld_2; /* (1) TCP:flags, ICMP:code */
    /// } __attribute__((__packed__)) pidx_data_t; 

	class PDXRecord : public Serializable <PDXRecord> {
	  /// \brief A typedef for the size_type of the serialized data
	  typedef std::string::size_type size_type;

	  private:
	  	TimeStamp base_time;
  		TimeStamp packet_timestamp;
		TimeStamp flow_timestamp;
		pidx_data_flags_t flags;
		uint32_t hash;
		uint16_t port;				/* (2) Ephemeral port (higher number) */
		uint8_t fld_1;				/* (1) ICMP:type */
		uint8_t fld_2;				/* (1) TCP:flags, ICMP:code */

	  public:

		PDXRecord();

		PDXRecord(TimeStamp* b_time);

		/// \brief Virtual destructor
		virtual ~PDXRecord();

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

		/// \returns time of packet hour.
		const TimeStamp& getBaseTime() const;

		/// \returns time packets recorded.
		const TimeStamp& getPacketTime() const;

		/// \returns time the flow started
		const TimeStamp& getFlowTime() const;

        /// \brief Returns the flags field as is from the struct as is. This 
        ///  is a mix of ARL and TCP/IP flags. Structured as follows:
        ///
        ///  Integer - 4 bytes:
        ///   31:   Client 1/0     - 1 indicates that we are looking at the client.
        ///   30:   Fragment 1/0   - 1 indicates we are looking at a fragment.
        ///   29:   Unused/reserved
        ///   28:     "
        ///   27:     "
        ///   26:   IP Flags
        ///   25:     "
        ///   24:     "
        ///   23 - 16:   TTL       - IP packet ttl.
        ///   15 - 0 :   Data Size - Number of bytes in the TCP/UDP packet.
        ///
        ///   At the moment there is no Vivic specific class to access this. The fields can
        ///   be directly accessed via bitwise ops or getPacketFlags().f.[field name].
		/// \returns packet flags
		const pidx_data_flags_t& getPacketFlags() const;

		/// \returns unique packet hash. A unique number that maps this packet to a 
        ///          NetFlow record. Used in order to find the Flow ID.
		const uint32_t& getHash() const;

		/// \returns Ephemeral port (higher number) 
		const uint16_t& getPort() const;

		const uint8_t& getField1() const;

		const uint8_t& getField2() const;

                const std::string toString() const;
	};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
