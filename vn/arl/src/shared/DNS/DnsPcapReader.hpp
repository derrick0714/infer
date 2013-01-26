#ifndef DNS_PCAP_READER
#define DNS_PCAP_READER

#include <fstream>
#include <arpa/inet.h>

#include "DNSPacket.hpp"
#include "../PcapFileReader.hpp"
#include "../EnumeratedFileReader.hpp"
#include "../Frame.h"
#include "../EthernetFrame.h"
#include "../FileReader.hpp"
#include "../IPv4Datagram.hpp"
#include "../IPv6Packet.hpp"
#include "../Segment.hpp"
#include "../UDPSegment.hpp"
#include "../TimeStamp.h"

namespace vn {
    namespace arl {
        namespace shared {

            template <typename FileEnumeratorType>
            class DnsPcapReader : public FileReader <FileEnumeratorType, DNSPacket> {
            private:
                PcapFileReader <EthernetFrame> preader;
                TimeStamp base_time;

            public:
                explicit DnsPcapReader(const FileEnumeratorType& fEnum):
                FileReader<FileEnumeratorType, DNSPacket>(fEnum) {
                    preader.setFilter("udp port 53");
                }

                ~DnsPcapReader() {
                    if (preader.isOpen()) {
                        preader.close();
                    }
                }

                virtual boost::shared_ptr <Serializable <DNSPacket> > read() {
                    using namespace std;
                    using namespace boost;

                    // open the next file to read.
                    if (!preader.isOpen()) {
                        if (this->fIt == this->fEnum.end()) {
                            // Don't bother, no more files.
                            return shared_ptr<Serializable <DNSPacket> >();
                        }

                        const char* filename = (this->fIt)->file_string().c_str();
                        preader.open(*this->fIt);

                        if (!preader) {
                            this->_error = true;
                            this->_errorMsg = string(" Failed to open file ") + filename;

                            return shared_ptr<Serializable <DNSPacket> >();
                        }

                        this->fIt++;
                    }

                    //do the reading.
                    DNSPacket* newRecord = new DNSPacket();
                    Serializable <DNSPacket> * rec = newRecord;

                    EthernetFrame f;
                    IPv4Datagram <EthernetFrame> ip4;
                    IPv6Packet <EthernetFrame> ip6;
                    UDPSegment <IPv4Datagram <EthernetFrame> > udp4;
                    UDPSegment <IPv6Packet <EthernetFrame> > udp6;

                    if(preader.read(f)) {
                        using namespace boost::asio::ip;
                        
                        TimeStamp ts(f.pcapHeader()->ts);
                        address sip;
                        address dip;

                        if (ip4.frame(f) && udp4.datagram(ip4)) {
                            sip = address(ip4.sourceIP());
                            dip = address(ip4.destinationIP());

                            newRecord->assign(udp4.payload(), udp4.payloadSize(), ts, sip, dip);
                        } else if(ip6.frame(f) && udp6.datagram(ip6)) {
                            sip = address(ip6.sourceIP());
                            dip = address(ip6.destinationIP());

                            newRecord->assign(udp6.payload(), udp6.payloadSize(), ts, sip, dip);
                        }
                    }

                    if (preader.eof()) {
                        preader.close();

                        return this->read();
                    }

                    return shared_ptr<Serializable <DNSPacket> >(rec);
                }
            };
        } // namespace shared
    } // namespace arl
} // namespace vn

#endif

