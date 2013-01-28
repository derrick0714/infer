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
#include "oldHTTP.h"

using namespace std;

/* Available command-line options. */
enum { REQUESTS, RESPONSES, CLIENT_ADDRESS, SERVER_ADDRESS, HOST, USER_AGENT,
       PATH };

vector <pair <uint32_t, uint32_t> > clientAddressRanges, serverAddressRanges;
vector <string> paths;
bool showRequests = true, showResponses = true, haveHost = false,
     haveUserAgent = false;
string host, userAgent;

void printOldHTTPRecord(const OldHTTP &record) {
  static size_t index;
  static bool match;
  /* -req (show requests) option. */
  if (!showResponses && record.type() == 's') {
    return;
  }
  /* -res (show responses) option. */
  if (!showRequests && record.type() == 'q') {
    return;
  }
  /* -cA (client address) option. */
  if (clientAddressRanges.size() > 0) {
    match = false;
    for (index = 0; index < clientAddressRanges.size(); ++index) {
      if (record.sourceIP() >= clientAddressRanges[index].first &&
          record.sourceIP() <= clientAddressRanges[index].second) {
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
      if (record.destinationIP() >= serverAddressRanges[index].first &&
          record.destinationIP() <= serverAddressRanges[index].second) {
        match = true;
        break;
      }
    }
    if (!match) {
      return;
    }
  }
  /* -h (host) option. */
  if (haveHost && (record.type() != 'q' || record.host().find(host) == string::npos)) {
    return;
  }
  /* -u (user agent) option. */
  if (haveUserAgent &&
      (record.type() != 'q' || record.userAgent().find(userAgent) == string::npos)) {
    return;
  }
  /* -p (path) option. */
  if (paths.size() > 0 && record.type() == 'q') {
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
  cout << "Type:\t\t";
  if (record.type() == 'q') {
    cout << "Request";
  }
  else {
    cout << "Response";
  }
  cout << endl << "Time:\t\t"
       << getDisplayTime(record.time()) << endl
       << "Source IP:\t"
       << ipToText(record.sourceIP()) << endl
       << "Source Port:\t"
       << record.sourcePort() << endl
       << "Destination IP:\t"
       << ipToText(record.destinationIP()) << endl
       << "Destination Port:\t"
       << record.destinationPort() << endl;
  if (record.type() == 'q') {
    if (!record.host().empty()) {
      cout << "Host:\t\t" << record.host() << endl;
    }
    cout << "Request Type:\t" << record.requestType() << endl
         << "Path:\t\t" << record.uri() << endl
         << "OldHTTP Version:\t" << record.version() << endl;
    if (!record.userAgent().empty()) {
      cout << "User Agent:\t" << record.userAgent() << endl;
    }
    if (!record.referer().empty()) {
      cout << "Referer:\t" << record.referer() << endl;
    }
  }
  else {
    cout << "OldHTTP Version:\t" << record.version() << endl
         << "Status Code:\t" << record.status() << ' ' << record.reason() << endl;
    if (!record.response().empty()) {
      cout << "Server Type:\t" << record.response() << endl;
    }
    if (!record.contentType().empty()) {
      cout << "Content type:\t" << record.contentType() << endl;
    }
  }
  cout << endl;
}

void usage(const char *programName) {
  cerr << "usage: " << programName << " [-req] [-resp] [-cA client address] "
       << "[-sA server address] [-h host] [-u user agent] [-p path] file ..."
       << endl;
}

int main(int argc, char *argv[]) {
  Options options(argc, argv, "req res cA: sA: h: u: p:");
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
     case REQUESTS:
        if (!showRequests) {
          cerr << argv[0] << ": " << "the \"-req\" and \"-res\" options are "
               << "mutually exclusive" << endl;
          return 1;
        }
        showResponses = false;
        break;
     case RESPONSES:
       if (!showResponses) {
         cerr << argv[0] << ": " << "the \"-req\" and \"-res\" options are "
               << "mutually exclusive" << endl;
          return 1;
        }
        showRequests = false;
        break;
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
  FlatFileReader<OldHTTP> reader;
  OldHTTP http;
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
	  printOldHTTPRecord(http);
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
