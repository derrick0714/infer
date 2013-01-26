#ifndef INFER_INCLUDE_LIVEIP_HPP_
#define INFER_INCLUDE_LIVEIP_HPP_

#include <netinet/in.h>
#include <boost/array.hpp>

#include <DataTypeTraits.hpp>

class LiveIP {
  public:
	typedef plain_old_data_tag data_type;

	static const uint8_t TypeID = 0x92;

	uint32_t rawIP() const {
		return *reinterpret_cast<const uint32_t*>(&_ip);
	}

	void rawIP(uint32_t rawIP) {
		*reinterpret_cast<uint32_t*>(&_ip) = rawIP;
	}

	uint32_t ip() const {
		return ntohl(rawIP());
	}

	void ip(uint32_t ip) {
		rawIP(htonl(ip));
	}

	boost::array<uint8_t, 6> mac() const {
		return _mac;
	}

	void mac(const boost::array<uint8_t, 6> &mac) {
		_mac = mac;
	}

  private:
	boost::array<uint8_t, 4> _ip;
	boost::array<uint8_t, 6> _mac;
};

#endif
