#ifndef ETHERNETADDRESS_H
#define ETHERNETADDRESS_H

#include <net/ethernet.h>
#include <string>
#include <ostream>

namespace vn {
namespace arl {
namespace shared {

class EthernetAddress;

} // namespace shared
} // namespace arl
} // namespace vn

std::ostream& operator<< (std::ostream &out,
						  const vn::arl::shared::EthernetAddress &addr);

namespace vn {
namespace arl {
namespace shared {

/// \brief An Ethernet Address
class EthernetAddress {
  public:
  	/// \brief Constructor
	///
	/// Construct an EthernetAddress from an unsigned character array.
	/// TODO use boost::array, perhaps?
	explicit EthernetAddress(const unsigned char *address);

	/// \brief Constructor
	///
	/// Construct an EthernetAddress from bytes stored in a std::string.
	explicit EthernetAddress(const std::string &address);

	/// \brief Get the bytes of this EthernetAddress
	/// \returns a string containing the bytes of this EthernetAddress
	std::string bytes() const;

	/// \brief Write this EthernetAddress to an std::ostream
	/// \param out The std::ostream to write to
	/// \param addr The EthernetAddress to be written
	///
	/// Writes addr to out in human readable format: XX:XX:XX:XX:XX:XX,
	/// where each XX is the hexadecimal representation of a byte in addr.
	friend std::ostream& ::operator<< (std::ostream &out,
									   const EthernetAddress &addr);

  private:
	/// An std::string to store the address
	const std::string address;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
