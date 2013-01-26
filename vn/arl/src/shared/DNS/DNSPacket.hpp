#ifndef DNSPACKET_HPP
#define DNSPACKET_HPP

#include <stdint.h>
#include <arpa/inet.h>
#include <vector>
#include <boost/asio/ip/address.hpp>

#include "../Serializable.hpp"
#include "../TimeStamp.h"
#include "DNSQueryEntry.h"
#include "DNSResponseEntry.h"
#include "../../debug/dbg.h"

namespace vn {
    namespace arl {
        namespace shared {

            using namespace std;
            using namespace boost::asio::ip;

            /// \brief DNS Packet
            ///
            /// This class represents a DNS version 4 packet.
            /// Parsing reference http://www.networksorcery.com/enp/protocol/dns.htm

            class DNSPacket : public Serializable <DNSPacket> {
            public:

                typedef enum _ResponseCode { NoError = 0, FormatError, ServerFailure, NameError, NotImplemented, Refused } ResponseCode;

                typedef enum _OpCode { Query = 0, IQuery, Status, Reserved3, Notify, Update, Reserved6, Reserved7, Reserved8,
                              Reserved9, Reserved10, Reserved11, Reserved12, Reserved13, Reserved14, Reserved15} OpCode;

                /// \brief Constructor
                DNSPacket() : dnsData(NULL){
                }

                ~DNSPacket() {
                    if(dnsData != NULL) {
                        delete dnsData;
                        dnsData = NULL;
                    }
                }

                /// \brief Assign the contained Datagram
                /// \param datagram the datagram to assign
                /// \returns true if the assignment was successful, false if datagram is
                /// not a DNS Packet
                bool assign(const unsigned char* data, int size, const TimeStamp& _timestamp, const address& sip, const address& dip) {
                    if(size > 16*1024 || size < 0){
                        return false;
                    }

                    packet_src_ip = sip;
                    packet_dst_ip = dip;
                    timestamp = _timestamp;
                    dnsData = new unsigned char[size];
                    memcpy(dnsData, data, size);

                    unsigned char* dataBegin = (dnsData + 12);
                    unsigned char* dataEdge = (dnsData + size);

                    //read queries.
                    int q_count = getQuestionCount();
                    for(int i = 0; i < q_count; i++) {
                        DNSQueryEntry dq;
                        vector<string> labels;

                        int byteConsumed = this->fillLabelsList(dataBegin, dataEdge, labels);
                        if(byteConsumed == -1) {
                            return false;
                        }

                        dataBegin += byteConsumed;
                        // yes it advances the pointers forward
                        unsigned short type = ((unsigned short)*dataBegin++) << 8 | *dataBegin++;
                        unsigned short klass = ((unsigned short)*dataBegin++) << 8 | *dataBegin++;

                        dq.assign(type, klass, labels);

                        queries.push_back(dq);
                    }

                    // read responses
                    int a_count = getAnswerCount();
                    for(int i = 0; i < a_count; i++) {
                        DNSResponseEntry da;
                        vector<string> labels;

                        int byteConsumed = this->fillLabelsList(dataBegin, dataEdge, labels);
                        if(byteConsumed == -1) {
                            return false;
                        }

                        dataBegin += byteConsumed;
                        // yes it advances the pointers forward
                        unsigned short type = ntohs(*(unsigned short*)dataBegin);
                        dataBegin += 2;

                        unsigned short klass = ntohs(*(unsigned short*)dataBegin);
                        dataBegin += 2;

                        unsigned int ttl = ntohl(*(uint32_t*)dataBegin);
                        dataBegin += 4;

                        unsigned short rdlen = ntohs(*(unsigned short*)dataBegin);
                        dataBegin += 2;

                        da.assign(type, klass, ttl, rdlen, dataBegin, labels);

                        if(dataBegin + rdlen > dataEdge) {
                            return false;
                        }

                        answers.push_back(da);
                    }

                    // we are going to ignore other entries until they are all needed.

                    return true;
                }

                const address& getSourceIP() const {
                    return packet_src_ip;
                }

                const address& getDestinationIP() const {
                    return packet_dst_ip;
                }

                unsigned short getID() {
                    return ntohs( *(unsigned short*)dnsData );
                }

                bool isResponse() {
                    return *(dnsData + 2) && 0x80;
                }

                OpCode getOPCode() {
                    return static_cast<OpCode>( (*(dnsData + 2) & 0x78) >> 3 );
                }

                bool isAuthoritativeResponse() {
                    return *(dnsData + 2) & 0x4;
                }

                bool isTruncation() {
                    return *(dnsData + 2) & 0x2;
                }

                bool isRecursionDesired() {
                    return *(dnsData + 2) & 0x1;
                }

                bool isRecursionAvailable() {
                    return *(dnsData + 3) & 0x80;
                }

                bool isZ() {
                    return *(dnsData + 3) & 0x40;
                }

                bool isAuthenticatedData() {
                    return *(dnsData + 3) & 0x20;
                }

                bool isCheckingDisabled() {
                    return *(dnsData + 3) & 0x10;
                }

                ResponseCode getResponseCode() {
                    return static_cast<ResponseCode>( *(dnsData + 3) & 0xF );
                }

                unsigned short getQuestionCount() {
                    return ntohs( *((unsigned short*)(dnsData + 4)) );
                }

                unsigned short getAnswerCount() {
                    return ntohs( *((unsigned short*)(dnsData + 6)) );
                }

                unsigned short getNameServerCount() {
                    return ntohs( *((unsigned short*)(dnsData + 8)) );
                }

                unsigned short getAdditionalRecordsCount() {
                    return ntohs( *((unsigned short*)(dnsData + 10)) );
                }

                string getDnsName(const vector<string>& list) {
                    string ret;

                    for(vector<string>::const_iterator it = list.begin(); it != list.end(); it++) {
                        ret.append(*it);
                        ret.append(".");
                    }

                    return ret;
                }

                const vector<DNSQueryEntry>& getQueries() {
                    return queries;
                }

                const vector<DNSResponseEntry>& getAnswers() {
                    return answers;
                }

                virtual TimeStamp startTime() const {
                    return timestamp;
                }

                /// \brief Get the end time of the data
                /// \returns the end time of the data
                virtual TimeStamp endTime() const {
                    return timestamp;
                }

                /// \brief Get the size of the the serialized data
                /// \returns the size of the serialized data
                virtual size_type size() const {
                    return 0;
                }

                /// \brief Serialize data
                /// \param ostr the string in which to store the serialized data
                /// \returns true if the data was successfully serialized into ostr
                virtual bool serialize(std::string &ostr) const {
                    return false;
                }

                /// \brief Unserialize data
                /// \param istr the string from which unserialize data
                /// \returns true if the data was successfully unserialized from ostr
                virtual bool unserialize(const std::string &istr) {
                    return false;
                }

            private:
                unsigned char* dnsData;
                TimeStamp timestamp;
                address packet_src_ip;
                address packet_dst_ip;

                vector<DNSQueryEntry> queries;
                vector<DNSResponseEntry> answers;

                /// \brief method on parse labels.
                /// \returns the count of bytes consumed (pointer aware).
                int fillLabelsList(unsigned char* data, unsigned char* edge, vector<string>& list) {
                    int len = (*data) & 0xFF;

                    if(len == 0) {
                        return 1;
                    } else if(*data & 0xC0) {
                        int offset = ((int)(*data & 0xC)) << 8 | (int)*(data + 1);
                        
                        if(offset < 12 || data + offset > edge) {
                            return -1;
                        }

                        fillLabelsList(dnsData + offset, edge, list);

                        return 2;
                    }
                    
                    string label((const char*)data + 1, (size_t)len);
                    list.push_back(label);

                    return 1 + len + fillLabelsList(data + 1 + len, edge, list);
                }
            };
        }
    }
}

#endif