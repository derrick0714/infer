#include <boost/tokenizer.hpp>

#include "HTTP.h"

namespace vn {
namespace arl {
namespace shared {

// not implemented yet
bool HTTP::serialize(std::string &) const {
	return false;
}

bool HTTP::unserialize(const std::string &data) {
	protocol(6); // all http data is TCP
	rawSourceIP(*(reinterpret_cast<const uint32_t*>(data.data())));
	rawDestinationIP(*(reinterpret_cast<const uint32_t*>(data.data() +
											sizeof(rawSourceIP()))));
	rawSourcePort(*(reinterpret_cast<const uint16_t*>(data.data() +
											sizeof(rawSourceIP()) +
											sizeof(rawDestinationIP()))));
	rawDestinationPort(*(reinterpret_cast<const uint16_t*>(data.data() +
											sizeof(rawSourceIP()) +
											sizeof(rawDestinationIP()) +
											sizeof(rawSourcePort()))));
	_time.set(*(reinterpret_cast<const uint32_t*>(data.data() +
											sizeof(rawSourceIP()) +
											sizeof(rawDestinationIP()) +
											sizeof(rawSourcePort()) +
											sizeof(rawDestinationPort()))),
			  *(reinterpret_cast<const uint32_t*>(data.data() +
											sizeof(rawSourceIP()) +
											sizeof(rawDestinationIP()) +
											sizeof(rawSourcePort()) +
											sizeof(rawDestinationPort()) +
											sizeof(uint32_t))));
	_type = *(data.data() + sizeof(rawSourceIP()) +
							sizeof(rawDestinationIP()) +
							sizeof(rawSourcePort()) +
							sizeof(rawDestinationPort()) +
							sizeof(TimeStamp));

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

	boost::char_separator<char> sep("\n", "", boost::keep_empty_tokens);
	std::string foo(data.substr(21));
	tokenizer tokens(foo, sep);
	tokenizer::iterator tok_iter(tokens.begin());

	switch (_type) {
	  case 'q':
		_requestType = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_uri = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_version = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_host = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_userAgent = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_referer = *tok_iter;
		/*
		++tok_iter;
		if (tok_iter != tokens.end()) {
			return false;
		}
		*/
		break;

	  case 's':
		_version = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_status = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_reason = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_response = *tok_iter;
		++tok_iter;
		if (tok_iter == tokens.end()) {
			return false;
		}
		_contentType = *tok_iter;
		/*
		++tok_iter;
		if (tok_iter != tokens.end()) {
			return false;
		}
		*/
		break;
	  default:
		// error
		return false;
		break;
	}

	return true;
}

} // namespace shared
} // namespace arl
} // namespace vn
