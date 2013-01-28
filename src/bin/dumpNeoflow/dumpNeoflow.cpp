#include <iostream>
#include <unistd.h>
#include <errno.h>

#include "options.h"
#include "timeStamp.h"
#include "sqlTime.h"
#include "hostPair.hpp"
#include "address.h"
#include "FlowStats.hpp"
#include "FlatFileReader.hpp"
#include "print_error.hpp"

using namespace std;

enum { SOURCE_ADDRESS, DESTINATION_ADDRESS };

pair <uint32_t, uint32_t> sourceAddressRange = make_pair(0, numeric_limits <uint32_t>::max()),
                          destinationAddressRange = make_pair(0, numeric_limits <uint32_t>::max());

string dataTypes[9] = { "Plaintext", "BMP image", "WAV audio", "Compressed",
                        "JPEG image", "MP3 audio", "MPEG video", "Encrypted" };

inline FlowStats::ContentType makeContentType(uint8_t contentType) {
	switch (contentType) {
	  case 0:
		return FlowStats::PLAINTEXT_TYPE;
		break;
	  case 1:
		return FlowStats::BMP_IMAGE_TYPE;
		break;
	  case 2:
		return FlowStats::WAV_AUDIO_TYPE;
		break;
	  case 3:
		return FlowStats::COMPRESSED_TYPE;
		break;
	  case 4:
		return FlowStats::JPEG_IMAGE_TYPE;
		break;
	  case 5:
		return FlowStats::MP3_AUDIO_TYPE;
		break;
	  case 6:
		return FlowStats::MPEG_VIDEO_TYPE;
		break;
	  case 7:
		return FlowStats::ENCRYPTED_TYPE;
		break;
	  default:
		abort();
		break;
	}

	return FlowStats::CONTENT_TYPES;
}
inline void printFlow(const FlowStats &flowStats) {
  if (flowStats.sourceIP() >= sourceAddressRange.first && flowStats.sourceIP() <= sourceAddressRange.second &&
      flowStats.destinationIP() >= destinationAddressRange.first && flowStats.destinationIP() <= destinationAddressRange.second) {
    cout << "Start Time:\t" << getDisplayTime(flowStats.startTime()) << endl;
    if (flowStats.protocol() == 6) {
      cout << "SYN time:\t" << getDisplayTime(flowStats.firstSYNTime()) << endl
           << "SYN/ACK time:\t" << getDisplayTime(flowStats.firstSYNACKTime())
           << endl << "ACK time:\t" << getDisplayTime(flowStats.firstACKTime())
           << endl;
    }
    cout << "End Time:\t"
         << getDisplayTime(flowStats.endTime()) << endl;
    if (flowStats.numPackets() > 1) {
      cout << "Min. IAT:\t" << flowStats.minInterArrivalTime().seconds() << '.'
           << setw(6) << setfill('0')
           << flowStats.minInterArrivalTime().microseconds() << endl
           << "Max. IAT:\t" << flowStats.maxInterArrivalTime().seconds() << '.'
           << setw(6) << setfill('0')
           << flowStats.maxInterArrivalTime().microseconds() << endl;
    }
    cout << "Source IP:\t"
         << ntop(flowStats.sourceIP()) << endl
         << "Source Port:\t"
         << flowStats.sourcePort() << endl
         << "Dest. IP:\t"
         << ntop(flowStats.destinationIP()) << endl
         << "Dest. Port:\t"
         << flowStats.destinationPort() << endl
         << "Protocol:\t"
         << (int)flowStats.protocol() << endl
         << "TOS:\t\t"
         << (int)flowStats.typeOfService() << endl
         << "Min. TTL:\t"
         << (int)flowStats.minTTL() << endl
         << "Max. TTL:\t"
         << (int)flowStats.maxTTL() << endl
         << "Num. Fragments:\t"
         << flowStats.numFrags() << endl
         << "Num. Bytes:\t"
         << flowStats.numBytes() << endl
         << "Num. Packets:\t"
         << flowStats.numPackets() << endl
         << "Min. Pkt. Size:\t"
         << flowStats.minPacketSize() << endl
         << "Max. Pkt. Size:\t"
         << flowStats.maxPacketSize() << endl
         << "TCP SYNs:\t"
         << flowStats.tcpSYNs() << endl
         << "TCP ACKs:\t"
         << flowStats.tcpACKs() << endl
         << "TCP FINs:\t"
         << flowStats.tcpFINs() << endl
         << "TCP RSTs:\t"
         << flowStats.tcpRSTs() << endl
         << "TCP PUSHs:\t"
         << flowStats.tcpPUSHs() << endl
         << "TCP URGs:\t"
         << flowStats.tcpURGs() << endl
         << "pkt_0_256:\t"
         << flowStats.sizeDistribution(0) << endl
         << "pkt_257_512:\t"
         << flowStats.sizeDistribution(1) << endl
         << "pkt_513_768:\t"
         << flowStats.sizeDistribution(2) << endl
         << "pkt_769_1024:\t"
         << flowStats.sizeDistribution(3) << endl
         << "pkt_1025_1280:\t"
         << flowStats.sizeDistribution(4) << endl
         << "pkt_1281_1536:\t"
         << flowStats.sizeDistribution(5) << endl
         << "pkt_1537_1792:\t"
         << flowStats.sizeDistribution(6) << endl
         << "pkt_1793_2048:\t"
         << flowStats.sizeDistribution(7) << endl;
    for (uint8_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
      cout << dataTypes[contentType] << ":\t"
           << flowStats.content(makeContentType(contentType)) << endl;
    }
    cout << endl;
  }
}

void usage(const char *programName) {
  cerr << "usage: " << programName << " [-sA source address] "
       << "[-dA destination address] file ..." << endl;
}

int main(int argc, char **argv) {
  Options options(argc, argv, "sA: dA:");
  int option;
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }
  while ((option = options.getOption()) != -1) {
    if (!options) {
      print_error(argv[0], options.error());
      return 1;
    }
    switch (option) {
      case SOURCE_ADDRESS:
        sourceAddressRange = cidrToRange(options.getArgument());
        break;
      case DESTINATION_ADDRESS:
        destinationAddressRange = cidrToRange(options.getArgument());
        break;
    }
  }
  if (options.getIndex() == argc) {
    usage(argv[0]);
    return 1;
  }
  FlatFileReader<FlowStats> reader;
  FlowStats flowStats;
  for (int fileNumber = options.getIndex(); fileNumber < argc; ++fileNumber) { 
    if (access(argv[fileNumber], R_OK) != 0) {
      print_error(argv[0], argv[fileNumber], strerror(errno));
      if (fileNumber < (argc - 1)) {
        cout << endl;
      }
    }
    else {
	  if (reader.open(argv[fileNumber]) != E_SUCCESS) {
	  	cerr << "Error opening file: " << argv[fileNumber] << endl;
		return 1;
	  }
      while (reader.read(flowStats) == E_SUCCESS) {
        printFlow(flowStats);
      }
	  if (reader.close() != E_SUCCESS) {
	  	cerr << "Error closing reader!" << endl;
		return 1;
	  }
    }
  }
  return 0;
}
