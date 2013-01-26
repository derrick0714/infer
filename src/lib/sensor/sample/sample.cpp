#include <iostream>
#include <string>

#include "modules.h"
#include "network.h"

using namespace std;

static size_t packetCount(0);

extern "C" {

	int initialize(const configuration &conf,
				   const std::string &outputDirectory,
				   const std::string &moduleDirectory)
		return 0;
	}

	int processPacket(const Packet &packet) {
		++packetCount;
		return 0;
	}

	int flush() {
		cout << endl << "sample: packetCount: " << packetCount << endl << endl;
		return 0;
	}

	int finish() {
		return 0;
	}

}
