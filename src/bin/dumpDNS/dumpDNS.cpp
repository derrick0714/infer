#include <iostream>
#include <db44/db.h>

#include <boost/lexical_cast.hpp>

#include "timeStamp.h"
#include "sqlTime.h"
#include "hostPair.hpp"
#include "DNS.hpp"
#include "FlatFileReader.hpp"

#define A_RECORD		1
#define CNAME_RECORD	5
#define PTR_RECORD		12
#define MX_RECORD		15

#define NUM_SUPPORTED_TYPES	4

using namespace std;

static char resourceRecordTypes[16][6] = { "A", "NS", "MD", "MF", "CNAME",
                                           "SOA", "MB", "MG", "MR", "NULL",
                                           "WKS", "PTR", "HINFO", "MINFO",
                                           "MX", "TXT" };

uint16_t supportedResourceRecordTypes[NUM_SUPPORTED_TYPES] = { 1, 5, 12, 15 };

static bool isSupportedType(uint16_t resourceRecordType) {
	for (uint16_t index = 0; index < NUM_SUPPORTED_TYPES; ++index) {
		if (supportedResourceRecordTypes[index] == resourceRecordType) {
			return true;
		}
	}
	return false;
}

inline string getResourceRecord(const DNS::DNSResponse &response) {
	switch (response.type()) {
	  case A_RECORD:
		return ntop(ntohl(*reinterpret_cast<const uint32_t *>(response.resourceData().data())));
		break;
	  case MX_RECORD:
	    return response.resourceData().substr(2) + string(" / ") + boost::lexical_cast<string>(static_cast<int16_t>(ntohs(*reinterpret_cast<const uint16_t *>(response.resourceData().data()))));
		break;
	  case CNAME_RECORD:
	  case PTR_RECORD:
		return response.resourceData();
		break;
	  default:
	  	break;
	}

	return string();
}	

inline void printDNSRecord(const DNS &dns) {
	cout << "Client IP:\t" << ntop(dns.clientIP()) << endl
		 << "Server IP:\t" << ntop(dns.serverIP()) << endl
		 << "Query time:\t" << getDisplayTime(dns.queryTime()) << endl
		 << "Response time:\t" << getDisplayTime(dns.responseTime())
		 << endl << "Query:\t\t" << dns.queryName()
		 << " / " << resourceRecordTypes[dns.queryType() - 1] << endl
		 << "Num. responses:\t" << dns.responses().size() << endl;
	if (dns.responses().size()) {
		cout << "Responses:\t";
		for (std::vector<DNS::DNSResponse*>::const_iterator it(dns.responses().begin());
			 it != dns.responses().end();
			 ++it)
		{
			if (!isSupportedType((*it)->type())) {
				cout << "Warning: unsupported resource record type "
					 << (*it)->type()
					 << "; skipping." << endl << endl;
				return;
			}
			if (it != dns.responses().begin()) {
				cout << "\t\t";
			}
			cout << (*it)->name() << " / "
				 << resourceRecordTypes[(*it)->type() - 1] << " / "
				 << getResourceRecord(*(*it)) << endl;
		}
	}
	cout << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cerr << "usage: " << argv[0] << " file ..." << endl;
		return 1;
	}
	FlatFileReader<DNS> reader;
	DNS dns;
	ErrorStatus errorStatus;
	for (int fileNumber = 1; fileNumber < argc; ++fileNumber) {
		if (reader.open(argv[fileNumber]) != E_SUCCESS) {
			cerr << "Unable to open '" << argv[fileNumber] << "'" << endl;
			return 1;
		}
		while ((errorStatus = reader.read(dns)) == E_SUCCESS) {
			printDNSRecord(dns);
		}
		if (errorStatus != E_EOF) {
			cerr << "Error reading from '" << argv[fileNumber] << "'" << endl;
			return 1;
		}
		if (reader.close() != E_SUCCESS) {
			cerr << "Error closing '" << argv[fileNumber] << "'" << endl;
			return 1;
		}
	}
	return 0;
}
