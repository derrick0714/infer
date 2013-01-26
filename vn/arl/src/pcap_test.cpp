#include <iostream>
#include <iomanip>
#include <arpa/inet.h>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/shared_ptr.hpp>

#include "shared/RecursiveReadEnumerator.hpp"
#include "shared/PcapFileReader.hpp"
#include "shared/EnumeratedFileReader.hpp"
#include "shared/Frame.h"
#include "shared/EthernetFrame.h"
#include "shared/IPv4Datagram.hpp"
#include "shared/Segment.hpp"
#include "shared/UDPSegment.hpp"
#include "shared/TCPSegment.hpp"

using namespace std;
using namespace vn::arl::shared;


void printFrameInfo(const Frame &f) {
	static char timeBuf[26];
	static time_t t;
	
	t = static_cast <time_t>(f.time().seconds());
	ctime_r(&t, timeBuf);
	timeBuf[24] = '\0';
	cout << "Pcap Header Info:" << endl
		 << "  Time:                         "
			<< timeBuf << '.' << f.time().microseconds() << endl
		 << "  Frame Size:                   "
			<< f.frameSize() << endl
		 << "  Captured Size:                "
			<< f.capturedSize() << endl;
}

void printEthernetFrameInfo(const EthernetFrame &e) {
	cout << "Ethernet Frame Info:" << endl
		 << "  Source Ethernet Address:      "
			<< e.sourceEthernetAddress() << endl
		 << "  Destination Ethernet Address: "
			<< e.destinationEthernetAddress() << endl
		 << "  Ethernet Type:                " 
			<< hex << "0x" << setw(4) << setfill('0') 
			<< e.datagramType() << dec << endl
		 << "  Frame Header Size:            " 
			<< e.frameHeaderSize() << endl
		 << "  Payload Size:                 " 
			<< e.payloadSize() << endl;
}

template <typename _FrameType>
void printIPv4PacketInfo(IPv4Datagram <_FrameType> p) {
	cout << "IPv4 Datagram Info:" << endl
		 << "  Source IP Address:            "
			<< p.sourceIP() << endl
		 << "  Destination IP Address:       "
			<< p.destinationIP() << endl
		 << "  Type of Service:              "
			<< hex << "0x" << setw(2) << setfill('0')
			<< (int) p.typeOfService() << dec << endl
		 << "  Fragmented:                   "
			<< ((p.fragmented())?"YES":"NO") << endl
		 << "  TTL:                          "
			<< (int) p.ttl() << endl
		 << "  Protocol:                     "
			<< (int) p.protocol() << endl
		 << "  Datagram Header Size:         " 
			<< p.datagramHeaderSize() << endl
		 << "  Payload Size:                 " 
			<< p.payloadSize() << endl;
}
/*
void printIPv6PacketInfo(boost::shared_ptr <IPv6Packet <EthernetFrame> > p) {
	cout << "IPv6 Packet Info:" << endl
		 << "  Source IP Address:            "
			<< p -> sourceIP() << endl
		 << "  Destination IP Address:       "
			<< p -> destinationIP() << endl;
//		 << "  Traffic Class  :              "
//			<< hex << "0x" << setw(2) << setfill('0')
//			<< (int) p -> trafficClass() << dec << endl
//		 << "  Fragmented:                   "
//			<< ((p -> fragmented())?"YES":"NO") << endl
//		 << "  Hop Limit                     "
//			<< (int) p -> hopLimit() << endl
//		 << "  Protocol:                     "
//			<< (int) p -> protocol() << endl
//		 << "  Packet Header Size:           " 
//			<< p -> packetHeaderSize() << endl;
}
*/

template <typename _DatagramType>
void printTCPSegmentInfo(TCPSegment <_DatagramType> s)
{
	cout << "TCP Segment Info:" << endl
		 << "  Source Port:                  "
			<< s.sourcePort() << endl
		 << "  Destination Port:             "
			<< s.destinationPort() << endl
		 << "  Sequence Number:              "
			<< s.sequenceNumber() << endl
		 << "  Acknowledgement Number:       "
			<< s.acknowledgementNumber() << endl
		 << "  Flags:                        "
			<< (s.cwr()?"CWR ":"")
			<< (s.ece()?"ECE ":"")
			<< (s.urg()?"URG ":"")
			<< (s.ack()?"ACK ":"")
			<< (s.psh()?"PSH ":"")
			<< (s.rst()?"RST ":"")
			<< (s.syn()?"SYN ":"")
			<< (s.fin()?"FIN":"") << endl
		 << "  Window Size:                  "
			<< s.windowSize() << endl
		 << "  Checksum:                     "
			<< hex << "0x" << setw(4) << setfill('0') 
			<< s.tcpChecksum() << dec << endl
		 << "  Segment Header Size:          " 
			<< s.segmentHeaderSize() << endl
		 << "  Payload Size:                 " 
			<< s.payloadSize() << endl;
}

template <typename _DatagramType>
void printUDPSegmentInfo(UDPSegment <_DatagramType> s)
{
	cout << "UDP Segment Info:" << endl
		 << "  Source Port:                  "
			<< s.sourcePort() << endl
		 << "  Destination Port:             "
			<< s.destinationPort() << endl
		 << "  UDP Length:                   "
			<< s.udpLength() << endl
		 << "  Checksum:                     "
			<< hex << "0x" << setw(4) << setfill('0') 
			<< s.udpChecksum() << dec << endl
		 << "  Segment Header Size:          " 
			<< s.segmentHeaderSize() << endl
		 << "  Payload Size:                 " 
			<< s.payloadSize() << endl;
}


int main(int argc, char **argv) {
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " dir_with_pcap_files" << endl;
		return 1;
	}

	boost::shared_ptr<RecursiveReadEnumerator> fEnum
		(new RecursiveReadEnumerator(argv[1]));
	if (!(*fEnum.get())) {
		cerr << "Error initializing RecursiveReadEnumerator. Exiting..."
			 << endl;
		return 1;
	}
	EnumeratedFileReader
		<PcapFileReader
			<EthernetFrame>, 
		 RecursiveReadEnumerator
	> enumReader(fEnum);

	EthernetFrame f;
	IPv4Datagram <EthernetFrame> ip4;
	UDPSegment <IPv4Datagram <EthernetFrame> > udp4;
	TCPSegment <IPv4Datagram <EthernetFrame> > tcp4;
	size_t frameCount(0);
	while (enumReader.read(f)) {
		++frameCount;
		cout << endl;

		printFrameInfo(Frame(f));
		printEthernetFrameInfo(f);
		if (!ip4.frame(f)) {
			continue;
		}
		printIPv4PacketInfo(ip4);

		if (udp4.datagram(ip4)) {
			printUDPSegmentInfo(udp4);
		} else if (tcp4.datagram(ip4)) {
			printTCPSegmentInfo(tcp4);
		}
	}
	if (!enumReader) {
		// read stopped because an error occurred.
		cerr << "Reader error: "
			 << enumReader.error() << endl;
		return 1;
	}

	cout << "# Frames read: " << frameCount << endl;

	return 0;
}
