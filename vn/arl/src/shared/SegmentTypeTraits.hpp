#ifndef SEGMENTTYPETRAITS_HPP
#define SEGMENTTYPETRAITS_HPP

namespace vn {
namespace arl {
namespace shared {

typedef enum {
	SEGMENT_TCP,
	SEGMENT_UDP,
	SEGMENT_UNKNOWN
} SegmentType;

struct segment_type_tag {
	virtual ~segment_type_tag() {}
};

struct udp_segment_type_tag : public segment_type_tag {};

struct tcp_segment_type_tag : public segment_type_tag {};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
