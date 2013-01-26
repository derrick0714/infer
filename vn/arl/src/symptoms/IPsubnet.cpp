/* 
 * File:   IPNetworkMask.cpp
 * Author: Mike
 * 
 * Created on January 16, 2010, 1:37 PM
 */

#include "IPsubnet.h"

#include <iostream>
#include <sstream>
#include <string>

namespace vn {
    namespace arl {
	namespace symptom {

	    IPsubnet::IPsubnet(const string& desc):error(false) {
		using namespace std;
		using namespace boost::asio::ip;

		address addr;
		int slash = desc.find('/');
		string ip = desc.substr(0, slash);
		istringstream mask(desc.substr(slash + 1));

		mask >> bits;
		try {
		    addr = address::from_string(ip);
		    isIPv6 = addr.is_v6();
		} catch ( ... ) {
		    error = true;
		    err_str.assign("Error parsing IP string '").append(ip).append("'");

		    return;
		}

		int ipBitsCount;

		if(isIPv6) {
		    ipBitsCount = sizeof(unsigned char) * 8 * boost::asio::ip::address_v6::bytes_type::static_size;
		    addr6 = addr.to_v6().to_bytes();
		} else {
		    ipBitsCount = sizeof(unsigned char) * 8 * boost::asio::ip::address_v4::bytes_type::static_size;
		    addr4 = addr.to_v4().to_bytes();
		}
		
		if(bits < 0 || bits > ipBitsCount) {
		    error = true;
		    err_str.assign("Bit value is invalid");

		    return;
		}
	    }

	    bool IPsubnet::errored() {
		return error;
	    }

	    const string& IPsubnet::getErrorStr() {
		return err_str;
	    }

	    bool IPsubnet::inRange(unsigned char* local, unsigned char* check, int size) {
		if(error) {
		    return false;
		}

		int elem_bit_size = sizeof(unsigned char) * 8;
		int back_bits = bits % elem_bit_size;
		int front_bytes = (bits - back_bits) / elem_bit_size;
		int i;

		for(i = 0; i < front_bytes; i++) {
		    if(local[i] != check[i]) {
			return false;
		    }
		}

		// don't do any work if we fell on the byte boundary (ie. /24).
		if(back_bits != 0) {
		    // fix up, our bits at the back are in the front of the byte boundary.
		    //  so we need to shift by the inverse.
		    back_bits = elem_bit_size - back_bits;

		    if(i <= size && (local[i] >> back_bits) != (check[i] >> back_bits)) {
			return false;
		    }
		}

		return true;
	    }

	    bool IPsubnet::inRange(const boost::asio::ip::address_v4& check) {
		if(isIPv6) {
		    // ensure we are comparing apples to apples, same IP versions.
		    return false;
		}

		boost::asio::ip::address_v4::bytes_type c_addr4 = check.to_bytes();
		unsigned char* localIpBytes = addr4.elems;
		unsigned char* checkIpBytes = c_addr4.elems;

		return inRange(localIpBytes, checkIpBytes, boost::asio::ip::address_v4::bytes_type::static_size);
	    }

	    bool IPsubnet::inRange(const boost::asio::ip::address_v6& check) {
		if(!isIPv6) {
		    // ensure we are comparing apples to apples, same IP versions.
		    return false;
		}

		boost::asio::ip::address_v6::bytes_type c_addr6 = check.to_bytes();
		unsigned char* localIpBytes = addr6.elems;
		unsigned char* checkIpBytes = c_addr6.elems;

		return inRange(localIpBytes, checkIpBytes, boost::asio::ip::address_v6::bytes_type::static_size);
	    }

	    bool IPsubnet::inRange(const boost::asio::ip::address& check) {
		if(check.is_v6()) {
		    return inRange(check.to_v6());
		} else {
		    return inRange(check.to_v4());
		}
	    }
	}
    }
}