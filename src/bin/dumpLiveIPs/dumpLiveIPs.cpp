#include <iostream>
#include <iomanip>

#include "LiveIP.hpp"
#include "FlatFileReader.hpp"
#include "hostPair.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "usage: " << argv[0] << " file ..." << endl;
    return 1;
  }
  LiveIP liveIP;
  FlatFileReader <LiveIP> reader;
  ErrorStatus errorStatus;
  boost::array<uint8_t, 6> mac;
  for (int fileNumber = 1; fileNumber < argc; ++fileNumber) {
  	if (reader.open(argv[fileNumber]) != E_SUCCESS) {
		cerr << "Unable to open '" << argv[fileNumber] << "'" << endl;
		return 1;
	}
	while ((errorStatus = reader.read(liveIP)) == E_SUCCESS) {
      cout << ntop(liveIP.rawIP(), false) << " (";
	  mac = liveIP.mac();
	  for (size_t i(0); i < mac.size(); ++i) {
	  	cout << hex << setw(2) << setfill('0') << (unsigned int) mac[i];
		if (i != mac.size() - 1) {
			cout << ':';
		}
	  }
	  cout << ')' << endl;
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
