#ifndef FRAMETYPETRAITS_HPP
#define FRAMETYPETRAITS_HPP

namespace vn {
namespace arl {
namespace shared {

typedef enum {
	FRAME_ETHERNET,
	FRAME_UNKNOWN
} FrameType;

struct frame_type_tag {
	virtual ~frame_type_tag() {}
};

struct ethernet_frame_type_tag : public frame_type_tag {};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
