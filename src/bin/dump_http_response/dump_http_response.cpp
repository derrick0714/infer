#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <unistd.h>
#include <errno.h>

#include "options.h"
#include "address.h"
#include "timeStamp.h"
#include "stringHelpers.h"
#include "sqlTime.h"
#include "FlatFileReader.hpp"
#include "HTTPResponse.h"

using namespace std;

/* Available command-line options. */
enum { CLIENT_ADDRESS, SERVER_ADDRESS };

vector <pair <uint32_t, uint32_t> > clientAddressRanges, serverAddressRanges;

void printHTTPResponseRecord(const HTTPResponse &record) {
  static size_t index;
  static bool match;
  /* -cA (client address) option. */
  if (clientAddressRanges.size() > 0) {
    match = false;
    for (index = 0; index < clientAddressRanges.size(); ++index) {
      if (record.destination_ip() >= clientAddressRanges[index].first &&
          record.destination_ip() <= clientAddressRanges[index].second) {
        match = true;
        break;
      }
    }
    if (!match) {
      return;
    }
  }
  /* -sA (server address) option. */
  if (serverAddressRanges.size() > 0) {
    match = false;
    for (index = 0; index < serverAddressRanges.size(); ++index) {
      if (record.source_ip() >= serverAddressRanges[index].first &&
          record.source_ip() <= serverAddressRanges[index].second) {
        match = true;
        break;
      }
    }
    if (!match) {
      return;
    }
  }
  cout << left << setw(18) << "Time:"
       << getDisplayTime(record.time()) << endl
       << left << setw(18) << "Source IP:"
       << ipToText(record.source_ip()) << endl
       << left << setw(18) << "Source Port:"
       << record.source_port() << endl
       << left << setw(18) << "Destination IP:"
       << ipToText(record.destination_ip()) << endl
       << left << setw(18) << "Destination Port:"
       << record.destination_port() << endl;
  cout << left << setw(18) << "HTTP Version:" << record.version() << endl
       << left << setw(18) << "Status Code:" << record.status() << ' ' << record.reason() << endl;
  if (!record.response().empty()) {
    cout << left << setw(18) << "Server Type:" << record.response() << endl;
  }
  if (!record.content_type().empty()) {
    cout << left << setw(18) << "Content type:" << record.content_type() << endl;
  }
  cout << endl;
}

void usage(const char *programName) {
  cerr << "usage: " << programName << " [-cA client address] "
       << "[-sA server address] file ..."
       << endl;
}

int main(int argc, char *argv[]) {
  Options options(argc, argv, "cA: sA:");
  int option;
  vector <string> files;
  bool error = false;
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }
  while ((option = options.getOption()) != -1) {
    if (!options) {
      cerr << argv[0] << ": " << options.error() << endl;
      return 1;
    }
    switch (option) {
     case CLIENT_ADDRESS:
        clientAddressRanges.push_back(cidrToRange(options.getArgument()));
        break;
      case SERVER_ADDRESS:
        serverAddressRanges.push_back(cidrToRange(options.getArgument()));
        break;
    }
  }
  if (options.getIndex() == argc) {
    usage(argv[0]);
    return 1;
  }
  for (int file = options.getIndex(); file < argc; ++file) { 
    if (access(argv[file], R_OK) != 0) {
      cerr << argv[0] << ": " << argv[file] << ": " << strerror(errno) << endl;
      error = true;
    }
    else {
      files.push_back(argv[file]);
    }
  }
  if (files.empty()) {
    return 1;
  }
  if (error) {
    cout << endl;
  }
  FlatFileReader<HTTPResponse> reader;
  HTTPResponse http;
  ErrorStatus errorStatus;
  for (vector<string>::iterator file(files.begin());
  	   file != files.end();
	   ++file)
  {
  	if (reader.open(*file) != E_SUCCESS) {
		cerr << "Unable to open '" << *file << "'" << endl;
		return 1;
	}
	while ((errorStatus = reader.read(http)) == E_SUCCESS) {
	  printHTTPResponseRecord(http);
	}
	if (errorStatus != E_EOF) {
		cerr << "Error reading from '" << *file << "'" << endl;
		return 1;
	}
	if (reader.close() != E_SUCCESS) {
		cerr << "Error closing '" << *file << "'" << endl;
		return 1;
	}
  }

  return 0;
}
