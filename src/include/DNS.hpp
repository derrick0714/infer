#ifndef INFER_INCLUDE_DNS_HPP_
#define INFER_INCLUDE_DNS_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <netinet/in.h>

#include <boost/numeric/conversion/cast.hpp>

#include "timeStamp.h"
#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

// FIXME TTL is not serialized/unserialized, as was the previous behavior
class DNS {
  public:
	typedef serializable_data_tag data_type;
	static const uint8_t TypeID = 0x93;

	class DNSResponse {
	  public:
	    std::string & name() {
			return _name;
		}

	    const std::string & name() const {
			return _name;
		}

		uint16_t type() const {
			return ntohs(_type);
		}

		void type(uint16_t type) {
			_type = htons(type);
		}

		uint16_t rawType() const {
			return _type;
		}

		void rawType(uint16_t rawType) {
			_type = rawType;
		}

		std::string & resourceData() {
			return _resourceData;
		}

		const std::string & resourceData() const {
			return _resourceData;
		}

		int32_t ttl() const {
			return ntohl(_ttl);
		}

		void ttl(int32_t ttl) {
			_ttl = htonl(ttl);
		}

		int32_t rawTTL() const {
			return _ttl;
		}

		void rawTTL(int32_t rawTTL) {
			_ttl = rawTTL;
		}

		ErrorStatus unserialize(std::istream &src) {
			uint8_t len;
			src.read(reinterpret_cast<char *>(&len), sizeof(len));
			if (!src) {
				return E_FSTREAM;
			}
			if (_name.size() == len) {
				_name.clear();
			}
			_name.resize(len);
			src.read(const_cast<char *>(_name.data()), len);
			if (!src) {
				return E_FSTREAM;
			}
			src.read(reinterpret_cast<char *>(&_type), sizeof(_type));
			if (!src) {
				return E_FSTREAM;
			}
			uint16_t len16;
			src.read(reinterpret_cast<char *>(&len16), sizeof(len16));
			if (!src) {
				return E_FSTREAM;
			}
			if (_resourceData.size() == len16) {
				_resourceData.clear();
			}
			_resourceData.resize(len16);
			src.read(const_cast<char *>(_resourceData.data()), len16);
			if (!src) {
				return E_FSTREAM;
			}

			return E_SUCCESS;
		}

	  private:
		std::string _name;
		uint16_t _type;
		std::string _resourceData;
		int32_t _ttl;
	};

	DNS()
		:_clientIP(0),
		 _serverIP(0),
		 _queryTime(),
		 _responseTime(),
		 _queryFlags(0),
		 _responseFlags(0),
		 _queryName(),
		 _queryType(0),
		 _responses()
	{}

	~DNS() {
		for (std::vector<DNSResponse*>::iterator it(_responses.begin());
			 it != _responses.end();
			 ++it)
		{
			delete *it;
		}
	}

	uint32_t clientIP() const {
		return ntohl(_clientIP);
	}

	void clientIP(uint32_t clientIP) {
		_clientIP = htonl(clientIP);
	}

	uint32_t rawClientIP() const {
		return _clientIP;
	}

	void rawClientIP(uint32_t rawClientIP) {
		_clientIP = rawClientIP;
	}

	uint32_t serverIP() const {
		return ntohl(_serverIP);
	}

	void serverIP(uint32_t serverIP) {
		_serverIP = htonl(serverIP);
	}

	uint32_t rawServerIP() const {
		return _serverIP;
	}

	void rawServerIP(uint32_t rawServerIP) {
		_serverIP = rawServerIP;
	}

	TimeStamp queryTime() const {
		return _queryTime;
	}

	void queryTime(const TimeStamp &queryTime) {
		_queryTime = queryTime;
	}

	TimeStamp responseTime() const {
		return _responseTime;
	}

	void responseTime(const TimeStamp &responseTime) {
		_responseTime = responseTime;
	}

	TimeStamp time() const {
		if (_queryTime.seconds()) {
			return _queryTime;
		}

		return _responseTime;
	}

	std::string & queryName() {
		return _queryName;
	}

	const std::string & queryName() const {
		return _queryName;
	}

	void queryName(const std::string &queryName) {
		_queryName = queryName;
	}

	uint16_t queryFlags() const {
		return ntohs(_queryFlags);
	}

	void queryFlags(uint16_t queryFlags) {
		_queryFlags = htons(queryFlags);
	}

	uint16_t rawQueryFlags() const {
		return _queryFlags;
	}

	void rawQueryFlags(uint16_t rawQueryFlags) {
		_queryFlags = rawQueryFlags;
	}

	uint16_t responseFlags() const {
		return ntohs(_responseFlags);
	}

	void responseFlags(uint16_t responseFlags) {
		_responseFlags = htons(responseFlags);
	}

	uint16_t rawResponseFlags() const {
		return _responseFlags;
	}

	void rawResponseFlags(uint16_t rawResponseFlags) {
		_responseFlags = rawResponseFlags;
	}

	uint16_t queryType() const {
		return ntohs(_queryType);
	}

	void queryType(uint16_t queryType) {
		_queryType = htons(queryType);
	}

	uint16_t rawQueryType() const {
		return _queryType;
	}

	void rawQueryType(uint16_t rawQueryType) {
		_queryType = rawQueryType;
	}

	std::vector<DNSResponse*> & responses() {
		return _responses;
	}

	const std::vector<DNSResponse*> & responses() const {
		return _responses;
	}

	ErrorStatus serialize(std::ostream &dst) const {
		uint16_t tmp16;

		dst.write(reinterpret_cast<const char *>(&_clientIP),
				  sizeof(_clientIP));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_serverIP),
				  sizeof(_serverIP));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_queryTime),
				  sizeof(_queryTime));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_responseTime),
				  sizeof(_responseTime));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_queryFlags),
				  sizeof(_queryFlags));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_responseFlags),
				  sizeof(_responseFlags));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.put(boost::numeric_cast<uint8_t>(_queryName.length()));
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(_queryName.data(), _queryName.length());
		if (!dst) {
			return E_FSTREAM;
		}
		dst.write(reinterpret_cast<const char *>(&_queryType),
				  sizeof(_queryType));
		if (!dst) {
			return E_FSTREAM;
		}
		tmp16 = boost::numeric_cast<uint16_t>(_responses.size());
		dst.write(reinterpret_cast<const char *>(&tmp16),
				  sizeof(tmp16));
		if (!dst) {
			return E_FSTREAM;
		}
		for (std::vector<DNSResponse*>::const_iterator it(_responses.begin());
			 it != _responses.end();
			 ++it)
		{
			dst.put(boost::numeric_cast<uint8_t>((*it)->name().length()));
			if (!dst) {
				return E_FSTREAM;
			}
			dst.write((*it)->name().data(), (*it)->name().length());
			if (!dst) {
				return E_FSTREAM;
			}
			tmp16 = (*it)->rawType();
			dst.write(reinterpret_cast<const char *>(&tmp16),
					  sizeof(tmp16));
			if (!dst) {
				return E_FSTREAM;
			}
			tmp16 = boost::numeric_cast<uint16_t>((*it)->resourceData().length());
			dst.write(reinterpret_cast<const char *>(&tmp16), sizeof(tmp16));
			if (!dst) {
				return E_FSTREAM;
			}
			dst.write((*it)->resourceData().data(), (*it)->resourceData().length());
			if (!dst) {
				return E_FSTREAM;
			}
		}

		return E_SUCCESS;
	}

	ErrorStatus unserialize(std::istream &src) {
		src.read(reinterpret_cast<char *>(&_clientIP),
				  sizeof(_clientIP));
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_serverIP),
				  sizeof(_serverIP));
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_queryTime),
				  sizeof(_queryTime));
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_responseTime),
				  sizeof(_responseTime));
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_queryFlags),
				  sizeof(_queryFlags));
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_responseFlags),
				  sizeof(_responseFlags));
		if (!src) {
			return E_FSTREAM;
		}
		uint8_t len;
		src.read(reinterpret_cast<char *>(&len), sizeof(len));
		if (!src) {
			return E_FSTREAM;
		}
		if (len == _queryName.size()) {
			_queryName.clear();
		}
		_queryName.resize(len);
		src.read(const_cast<char *>(_queryName.data()), len);
		if (!src) {
			return E_FSTREAM;
		}
		src.read(reinterpret_cast<char *>(&_queryType),
				  sizeof(_queryType));
		if (!src) {
			return E_FSTREAM;
		}
		
		for (std::vector<DNSResponse*>::iterator it(_responses.begin());
			 it != _responses.end();
			 ++it)
		{
			delete *it;
		}
		_responses.clear();

		uint16_t tmp16;
		src.read(reinterpret_cast<char *>(&tmp16),
				 sizeof(tmp16));
		if (!src) {
			return E_FSTREAM;
		}
		std::vector <DNS::DNSResponse*>::iterator it;
		for (uint16_t i(0); i < tmp16; ++i)
		{
			it = _responses.insert(_responses.end(), new DNS::DNSResponse);
			ErrorStatus status((*it)->unserialize(src));
			if (status != E_SUCCESS) {
				return status;
			}
		}

		return E_SUCCESS;
	}

	DNS(const DNS &dns)
		:_clientIP(dns._clientIP),
		 _serverIP(dns._serverIP),
		 _queryTime(dns._queryTime),
		 _responseTime(dns._responseTime),
		 _queryFlags(dns._queryFlags),
		 _responseFlags(dns._responseFlags),
		 _queryName(dns._queryName),
		 _queryType(dns._queryType),
		 _responses()
	{
		for (std::vector<DNSResponse*>::const_iterator i(dns._responses.begin());
			 i != dns._responses.end();
			 ++i)
		{
			_responses.push_back(new DNSResponse(*(*i)));
		}
	}

	const DNS & operator=(const DNS &dns) {
		_clientIP = dns._clientIP;
		_serverIP = dns._serverIP;
		_queryTime = dns._queryTime;
		_responseTime = dns._responseTime;
		_queryFlags = dns._queryFlags;
		_responseFlags = dns._responseFlags;
		_queryName = dns._queryName;
		_queryType = dns._queryType;
		
		for (std::vector<DNSResponse*>::iterator it(_responses.begin());
			 it != _responses.end();
			 ++it)
		{
			delete *it;
		}
		_responses.clear();
		for (std::vector<DNSResponse*>::const_iterator i(dns._responses.begin());
			 i != dns._responses.end();
			 ++i)
		{
			_responses.push_back(new DNSResponse(*(*i)));
		}

		return *this;
	}

  private:
	uint32_t _clientIP;
	uint32_t _serverIP;
	TimeStamp _queryTime;
	TimeStamp _responseTime;
	uint16_t _queryFlags;
	uint16_t _responseFlags;
	std::string _queryName;
	uint16_t _queryType;
	std::vector<DNSResponse*> _responses;
};

#endif
