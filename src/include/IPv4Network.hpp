#ifndef INFER_INCLUDE_IPV4NETWORK_HPP_
#define INFER_INCLUDE_IPV4NETWORK_HPP_

#include <netinet/in.h>

class IPv4Network {
  public:
	IPv4Network()
		:_network(0),
		 _netmask(0)
	{
	}

	bool set(uint32_t network, uint32_t netmask) {
		if ((network & netmask) != network) {
			return false;
		}

		_network = htonl(network);
		_netmask = htonl(netmask);

		return true;
	}

	bool rawSet(uint32_t network, uint32_t netmask) {
		if ((network & netmask) != network) {
			return false;
		}

		_network = network;
		_netmask = netmask;

		return true;
	}

	bool isInNetwork(uint32_t ipv4Address) const {
		return (htonl(ipv4Address) & _netmask) == _network;
	}

	bool rawIsInNetwork(uint32_t rawIPv4Address) const {
		return (rawIPv4Address & _netmask) == _network;
	}

	uint32_t network() const {
		return ntohl(_network);
	}

	uint32_t netmask() const {
		return ntohl(_netmask);
	}

	uint32_t rawNetwork() const {
		return _network;
	}

	uint32_t rawNetmask() const {
		return _netmask;
	}

  private:
	uint32_t _network;
	uint32_t _netmask;
};

#endif
