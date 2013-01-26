#include <iomanip>
#include <sys/types.h>
#include <net/ethernet.h>

#include "EthernetAddress.h"

namespace vn {
namespace arl {
namespace shared {

EthernetAddress::EthernetAddress(const unsigned char *address)
	:address((const char *) address, ETHER_ADDR_LEN)
{
}

EthernetAddress::EthernetAddress(const std::string &address)
	:address(address)
{
}

std::string EthernetAddress::bytes() const {
	return address;
}

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out,
						  const vn::arl::shared::EthernetAddress &address)
{
	std::ios_base::fmtflags oldFlags(out.flags());
	
	out << std::hex << std::setw(2) << std::setfill('0')
		<< (int) (unsigned char) address.address[0];
	for (std::string::size_type i = 1; i < address.address.size(); ++i) {
		out << ':' << std::setw(2) << std::setfill('0')
			<< (int) (unsigned char) address.address[i];
	}

	out.flags(oldFlags);

	return out;
}
