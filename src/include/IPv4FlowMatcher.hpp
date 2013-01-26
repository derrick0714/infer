#ifndef INFER_INCLUDE_IPV4FLOWMATCHER_HPP_
#define INFER_INCLUDE_IPV4FLOWMATCHER_HPP_

#include <iostream>

#include "IPv4Network.hpp"

class IPv4FlowMatcher {
	typedef std::vector <std::pair<uint8_t, uint8_t> > ProtocolRangesContainer;
	typedef std::vector <IPv4Network> NetworksContainer;
	typedef std::vector <std::pair<uint16_t, uint16_t> > PortRangesContainer;

  public:
	IPv4FlowMatcher()
		:_protocolRanges(),
		 _sourceNetworks(),
		 _destinationNetworks(),
		 _eitherNetworks(),
		 _sourcePortRanges(),
		 _destinationPortRanges(),
		 _eitherPortRanges(),
		 _error(false),
		 _errorMsg()
	{
	}

	IPv4FlowMatcher(const ProtocolRangesContainer &protocolRanges,
					const NetworksContainer &sourceNetworks,
					const NetworksContainer &destinationNetworks,
					const NetworksContainer &eitherNetworks,
					const PortRangesContainer &sourcePortRanges,
					const PortRangesContainer &destinationPortRanges,
					const PortRangesContainer &eitherPortRanges,
					bool error,
					const std::string &errorMsg)
		:_protocolRanges(protocolRanges),
		 _sourceNetworks(sourceNetworks),
		 _destinationNetworks(destinationNetworks),
		 _eitherNetworks(eitherNetworks),
		 _sourcePortRanges(sourcePortRanges),
		 _destinationPortRanges(destinationPortRanges),
		 _eitherPortRanges(eitherPortRanges),
		 _error(error),
		 _errorMsg(errorMsg)
	{
	}

	bool addProtocolRange(std::pair <uint8_t, uint8_t> protocolRange) {
		if (protocolRange.first > protocolRange.second) {
			_error = true;
			_errorMsg.assign("invalid protocol range");
			return false;
		}

		_protocolRanges.push_back(protocolRange);
		return true;
	}

	bool addSourceNetwork(const IPv4Network &ipv4Network) {
		if (!_eitherNetworks.empty()) {
			_error = true;
			_errorMsg.assign("sourceNetworks may not be used in conjunction "
							 "with eitherNetworks");
			return false;
		}

		_sourceNetworks.push_back(ipv4Network);
		return true;
	}

	bool addDestinationNetwork(const IPv4Network &ipv4Network) {
		if (!_eitherNetworks.empty()) {
			_error = true;
			_errorMsg.assign("destinationNetworks may not be used in "
							 "conjunction with eitherNetworks");
			return false;
		}

		_destinationNetworks.push_back(ipv4Network);
		return true;
	}

	bool addEitherNetwork(const IPv4Network &ipv4Network) {
		if (!_sourceNetworks.empty() || !_destinationNetworks.empty()) {
			_error = true;
			_errorMsg.assign("eitherNetworks may not be used in conjunction "
							 "with sourceNetworks or destinationNetworks");
			return false;
		}

		_eitherNetworks.push_back(ipv4Network);
		return true;
	}

	// ports must be in host byte order
	bool addSourcePortRange(std::pair <uint16_t, uint16_t> portRange) {
		if (portRange.first > portRange.second) {
			_error = true;
			_errorMsg.assign("invalid source port range");
			return false;
		}

		if (!_eitherPortRanges.empty()) {
			_error = true;
			_errorMsg.assign("sourcePortRanges cannot be used in conjunction "
							 "with eitherPortRanges");
			return false;
		}

		_sourcePortRanges.push_back(portRange);
		return true;
	}

	// ports must be in host byte order
	bool addDestinationPortRange(std::pair <uint16_t, uint16_t> portRange) {
		if (portRange.first > portRange.second) {
			_error = true;
			_errorMsg.assign("invalid destination port range");
			return false;
		}

		if (!_eitherPortRanges.empty()) {
			_error = true;
			_errorMsg.assign("destinationPortRanges cannot be used in "
							 "conjunction with eitherPortRanges");
			return false;
		}

		_destinationPortRanges.push_back(portRange);
		return true;
	}

	// ports must be in host byte order
	bool addEitherPortRange(std::pair <uint16_t, uint16_t> portRange) {
		if (portRange.first > portRange.second) {
			_error = true;
			_errorMsg.assign("invalid port range");
			return false;
		}

		if (!_sourcePortRanges.empty() || !_destinationPortRanges.empty()) {
			_error = true;
			_errorMsg.assign("eitherPortRanges may not be used in conjunction "
							 "with sourcePortRanges or destinationPortRanges");
			return false;
		}

		_eitherPortRanges.push_back(portRange);
		return true;
	}

	template <typename T>
	bool isMatch(const T &flow) const {
		static bool match;

		// check protocol against ranges
		match = true;
		if (!_protocolRanges.empty()) {
			match = false;
			for (ProtocolRangesContainer::const_iterator
					it(_protocolRanges.begin());
				 it != _protocolRanges.end();
				 ++it)
			{
				if (flow.protocol() >= it->first &&
					flow.protocol() <= it->second)
				{
					match = true;
					break;
				}
			}
		}
		if (!match) {
			return false;
		}

		// check sourceIP against networks
		if (!_sourceNetworks.empty()) {
			match = false;
			for (NetworksContainer::const_iterator
										it(_sourceNetworks.begin());
				 it != _sourceNetworks.end();
				 ++it)
			{
				if (it->rawIsInNetwork(flow.rawSourceIP())) {
					match = true;
					break;
				}
			}
		}
		if (!match) {
			return false;
		}

		// check destinationIP against networks
		if (!_destinationNetworks.empty()) {
			match = false;
			for (NetworksContainer::const_iterator
										it(_destinationNetworks.begin());
				 it != _destinationNetworks.end();
				 ++it)
			{
				if (it->rawIsInNetwork(flow.rawDestinationIP())) {
					match = true;
					break;
				}
			}
		}
		if (!match) {
			return false;
		}

		// check either IP against networks
		if (!_eitherNetworks.empty()) {
			match = false;
			for (NetworksContainer::const_iterator
										it(_eitherNetworks.begin());
				 it != _eitherNetworks.end();
				 ++it)
			{
				if (it->rawIsInNetwork(flow.rawSourceIP()) ||
					it->rawIsInNetwork(flow.rawDestinationIP())) 
				{
					match = true;
					break;
				}
			}
		}
		if (!match) {
			return false;
		}

		// check sourcePort against ranges
		if (!_sourcePortRanges.empty()) {
			match = false;
			for (PortRangesContainer::const_iterator
										it(_sourcePortRanges.begin());
				 it != _sourcePortRanges.end();
				 ++it)
			{
				if (flow.sourcePort() >= it->first &&
					flow.sourcePort() <= it->second)
				{
					match = true;
					break;
				}
			}
		}
		if (!match) {
			return false;
		}

		// check destinationPort against ranges
		if (!_destinationPortRanges.empty()) {
			match = false;
			for (PortRangesContainer::const_iterator
										it(_destinationPortRanges.begin());
				 it != _destinationPortRanges.end();
				 ++it)
			{
				if (flow.destinationPort() >= it->first &&
					flow.destinationPort() <= it->second)
				{
					match = true;
					break;
				}
			}
		}

		// check either port against ranges
		if (!_eitherPortRanges.empty()) {
			match = false;
			for (PortRangesContainer::const_iterator
										it(_eitherPortRanges.begin());
				 it != _eitherPortRanges.end();
				 ++it)
			{
				if ((flow.destinationPort() >= it->first &&
					 flow.destinationPort() <= it->second) ||
					(flow.sourcePort() >= it->first &&
					 flow.sourcePort() <= it->second))
				{
					match = true;
					break;
				}
			}
		}

		return match;
	}

	IPv4FlowMatcher reverse() const {
		return IPv4FlowMatcher(_protocolRanges,
							   _destinationNetworks,
							   _sourceNetworks,
							   _eitherNetworks,
							   _destinationPortRanges,
							   _sourcePortRanges,
							   _eitherPortRanges,
							   _error,
							   _errorMsg);
	}

  private:
	ProtocolRangesContainer _protocolRanges;

	NetworksContainer _sourceNetworks;
	NetworksContainer _destinationNetworks;
	NetworksContainer _eitherNetworks;

	PortRangesContainer _sourcePortRanges;
	PortRangesContainer _destinationPortRanges;
	PortRangesContainer _eitherPortRanges;

	bool _error;

	std::string _errorMsg;
};

#endif
