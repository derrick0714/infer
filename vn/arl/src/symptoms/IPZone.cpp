/* 
 * File:   IPZone.cpp
 * Author: Mike
 * 
 * Created on January 16, 2010, 3:55 PM
 */

#include <boost/tokenizer.hpp>

#include "IPZone.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace std;
	    using namespace boost;

	    const IPsubnet& IPZone::addAddressRangeList(const string& spec) {
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

		boost::char_separator<char> sep(" ");
		tokenizer tok(spec, sep);

		for (tokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		    IPsubnet sn(*beg);

		    ip_list.push_back(sn);

		    if (sn.errored()) {
			return ip_list.back();
		    }
		}

		return ip_list.back();
	    }

	    bool IPZone::inRange(const boost::asio::ip::address& check) const {
		if (check.is_v6()) {
		    return inRange(check.to_v6());
		} else {
		    return inRange(check.to_v4());
		}
	    }

	    bool IPZone::inRange(const boost::asio::ip::address_v4& check) const {
		vector<IPsubnet>::const_iterator iterator = ip_list.begin();
		
		do {
		    IPsubnet net = *iterator;
		    
		    if (net.inRange(check)) {
			return true;
		    }

		    iterator++;
		} while (iterator != ip_list.end());

		return false;
	    }

	    bool IPZone::inRange(const boost::asio::ip::address_v6& check) const {
		vector<IPsubnet>::const_iterator iterator = ip_list.begin();
		do {
		    IPsubnet net = *iterator;

		    if (net.inRange(check)) {
			return true;
		    }

		    iterator++;
		} while (iterator != ip_list.end());

		return false;
	    }

	    void IPZone::clear() {
		ip_list.clear();
	    }

	    int IPZone::getEntryCount() const {
		return ip_list.size();
	    }
	}
    }
}