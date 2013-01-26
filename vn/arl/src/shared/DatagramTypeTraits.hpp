#ifndef DATAGRAMTYPETRAITS_HPP
#define DATAGRAMTYPETRAITS_HPP

namespace vn {
namespace arl {
namespace shared {

typedef enum {
	DATAGRAM_IPV4,
        DATAGRAM_IPV6,
	DATAGRAM_UNKNOWN
} DatagramType;

struct datagram_type_tag {
	virtual ~datagram_type_tag() {}
};

struct ipv4_datagram_type_tag : public datagram_type_tag {};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
