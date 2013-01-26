/* 
 * File:   test_IPZone.cpp
 * Author: Mike
 *
 * Created on January 16, 2010, 5:25 PM
 *
 * this class is for testing the IPZone class.
 */

#include <stdlib.h>
#include <string>
#include <iostream>
#include <boost/asio/ip/address.hpp>
#include "symptoms/IPZone.h"
#include "symptoms/IPsubnet.h"

using namespace std;
using namespace vn::arl::symptom;
using namespace boost::asio::ip;

void test(const string& subnet, const string& test) {
    address addr = address::from_string(test);
    IPsubnet sn(subnet);

    if(sn.errored()) {
	cout << "Error with " + subnet << " (" << sn.getErrorStr() << ") " << endl;
    } else {
	cout << addr.to_string() << " is in " << subnet << " " << sn.inRange(addr) << endl;
    }
}

int main(int argc, char** argv) {
    string subnets[] = {
	string("10.1.2.3/32"),
	string("10.1.2.3/30"),
	string("10.1.2.3/24"),
	string("10.1.2.3/20"),
	string("10.1.2.3/16"),
	string("10.1.2.3/12"),
	string("10.1.2.3/10"),
	string("10.1.2.3/8"),
	string("10.1.2.3/4")
    };

    string testIPs[] = {
	string("10.1.2.3"),
	string("10.1.2.1"),
	string("10.1.2.255"),
	string("10.1.5.255"),
	string("10.1.255.255"),
	string("10.3.255.255"),
	string("10.3.255.255"),
	string("10.255.255.255"),
	string("8.255.255.30")
    };

    for(int i = 0; i < sizeof(subnets)/sizeof(string); i++) {
	for(int k = 0; k < sizeof(testIPs)/sizeof(string); k++) {
	    test(subnets[i], testIPs[k]);
	}
    }

    cout << "Test zone" << endl;
    string all;

    for(int i = 0; i < sizeof(subnets)/sizeof(string); i++) {
	all.append(subnets[i]).append(" ");
    }

    IPZone zone;
    IPsubnet sn = zone.addAddressRangeList(all);

    if(sn.errored()) {
	cout << "error: " << sn.getErrorStr() << endl;
	return (EXIT_SUCCESS);
    }

    cout << "Zone: " << all << endl;
    for(int k = 0; k < sizeof(testIPs)/sizeof(string); k++) {
	cout << testIPs[k] << " is in zone: " << zone.inRange(address::from_string(testIPs[k])) << endl;
    }

    return (EXIT_SUCCESS);
}

