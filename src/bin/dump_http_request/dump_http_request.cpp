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
#include "HTTPRequest.h"

using namespace std;

/* Available command-line options. */
enum { CLIENT_ADDRESS, SERVER_ADDRESS, HOST, USER_AGENT,
       PATH, REFERER };

vector <pair <uint32_t, uint32_t> > clientAddressRanges, serverAddressRanges;
vector <string> paths;
vector <string> referers;
bool showRequests = true, haveHost = false, haveUserAgent = false;
string host, userAgent;

void printHTTPRequestRecord(const HTTPRequest &record) {
  static size_t index;
  static bool match;
  /* -cA (client address) option. */
  if (clientAddressRanges.size() > 0) {
    match = false;
    for (index = 0; index < clientAddressRanges.size(); ++index) {
      if (record.source_ip() >= clientAddressRanges[index].first &&
          record.source_ip() <= clientAddressRanges[index].second) {
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
      if (record.destination_ip() >= serverAddressRanges[index].first &&
          record.destination_ip() <= serverAddressRanges[index].second) {
        match = true;
        break;
      }
    }
    if (!match) {
      return;
    }
  }
  /* -h (host) option. */
  if (haveHost && record.host().find(host) == string::npos) {
    return;
  }
  /* -u (user agent) option. */
  if (haveUserAgent && record.user_agent().find(userAgent) == string::npos) {
    return;
  }
  /* -p (path) option. */
  if (paths.size() > 0) {
    match = false;
    for (index = 0; index < paths.size(); ++index) {
      if (record.uri().find(paths[index]) != string::npos) {
        match = true;
        break;
      }
    }
    if (!match) {
      return;
    }
  }
  /* -r (path) option. */
  if (referers.size() > 0) {
    match = false;
    for (index = 0; index < referers.size(); ++index) {
      if (record.referer().find(referers[index]) != string::npos) {
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
    if (!record.host().empty()) {
      cout << left << setw(18) << "Host:" << record.host() << endl;
    }
    cout << left << setw(18) << "Request Type:" << record.type() << endl
         << left << setw(18) << "Path:" << record.uri() << endl
         << left << setw(18) << "HTTP Version:" << record.version() << endl;
    if (!record.user_agent().empty()) {
      cout << left << setw(18) << "User Agent:" << record.user_agent() << endl;
    }
    if (!record.referer().empty()) {
      cout << left << setw(18) << "Referer:" << record.referer() << endl;
    }
  cout << endl;
}

void usage(const char *programName) {
  cerr << "usage: " << programName << " -cA client address] "
       << "[-sA server address] [-h host] [-u user agent] [-p path] file ..."
       << endl;
}

int main(int argc, char *argv[]) {
  Options options(argc, argv, "cA: sA: h: u: p: r:");
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
      case HOST:
        haveHost = true;
        host = options.getArgument();
        break;
      case USER_AGENT:
        haveUserAgent = true;
        userAgent = options.getArgument();
        break;
      case PATH:
        paths.push_back(options.getArgument());
        break;
      case REFERER:
        referers.push_back(options.getArgument());
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
  FlatFileReader<HTTPRequest> reader;
  HTTPRequest http;
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
	  printHTTPRequestRecord(http);
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
