#ifndef PEER_TO_PEER_HPP
#define PEER_TO_PEER_HPP

#include "../../debug/dbg.h"

#include <math.h>

#include <iostream>
#include <map>

#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../../shared/arl_parsing/NetFlowRecords.h"
#include "../../shared/TimeStamp.h"
#include "../../shared/pmap.hpp"

#include "../IPZone.h"
#include "../Symptom.hpp"
#include "../NetflowEntry.h"
#include "PeerToPeerArguments.h"
#include "PeerToPeerMapEntry.h"

// 250MB segments.
#define MAP_SEGMENT_SIZE 250 * 1024 * 1024

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace std;
            using namespace vn::arl::shared;
            using namespace boost::posix_time;
            using namespace boost::asio::ip;

            template <typename ReaderType, typename WriterType>
            class PeerToPeerSymptom : public Symptom<PeerToPeerArguments> {
            private:

                ReaderType& reader;
                WriterType& writer;

                bool error;
                string errorString;

                NetFlowARL_v1_Record* rec1;
                NetFlowARL_v2_Record* rec2;
                NetFlowARL_v3_Record* rec3;

                void flag(NetFlowARLRecord& rec) {
                    // send to writer.
                    writer.write(NetflowEntry(rec));

                    this->arl_output(rec, string("P2P Node symptom"));
                }

                NetFlowARLRecord* getNetFlowObject(typename ReaderType::value_type& entry) {
                    switch(entry.getVersion()) {
                        case NetFlowARLRecord::NetFlow_v1:
                            entry.getNetFlow(*rec1);
                            return rec1;

                        case NetFlowARLRecord::NetFlow_v2:
                            entry.getNetFlow(*rec2);
                            return rec2;

                        case NetFlowARLRecord::NetFlow_v3:
                            entry.getNetFlow(*rec3);
                            return rec3;
                    }

                    return NULL;
                }

            public:

                explicit PeerToPeerSymptom(const PeerToPeerArguments &args, ReaderType &rdr, WriterType &wtr) :
                Symptom<PeerToPeerArguments>(args, "PeerToPeer", args.getSensorName()),
                reader(rdr), writer(wtr), error(false) {
                    rec1 = new NetFlowARL_v1_Record();
                    rec2 = new NetFlowARL_v2_Record();
                    rec3 = new NetFlowARL_v3_Record();
                }

                virtual bool isError() const {
                    return error;
                }

                virtual const string& getError() const {
                    return errorString;
                }

                virtual int run() {
                    if(error) {
                        return 1;
                    }

                    typedef PeerToPeerMapEntry MapTypeValue;
                    typedef map<address, MapTypeValue> MapType;
                    MapType darkConnectMap;

                    cout << "# Running Dark Access Symptom Detection." << endl;
                    
                    const IPZone& zone = this->s_args.getMonitoredZone();

                    const TimeStamp& start = this->s_args.getStartTime();
                    const TimeStamp& end = this->s_args.getEndTime();
                    const int ephPortStart = this->s_args.getEphemeralPortStart();
                    const int ephPortEnd = this->s_args.getEphemeralPortEnd();
                    const bool phase2 = this->s_args.isPhase2();

                    // this could probably be handled better, but we'll leave that for later.
                    NetFlowARLRecord* rec;

                    // Yes, we are expecting a NetflowEntry type here.
                    // if not... then we are probably screwed.
                    typename ReaderType::value_type entry;

                    while(reader.read(entry)) {
                        rec = getNetFlowObject(entry);
                        const TimeStamp& flowEndTime = rec->endTime();
                        const int serverPort = rec->getServerPort();

                        bool inZone = zone.getEntryCount() == 0 || zone.inRange(rec->getClientIP());
                        bool inTime = flowEndTime > start && flowEndTime < end;
                        bool ephPort = serverPort >= ephPortStart && serverPort <= ephPortEnd;

                        if(inZone && inTime) {
                            // we want to process this!

                            const address& clientAddr = rec->getClientIP();
                            MapType::iterator ment = darkConnectMap.find(clientAddr);

                            if(ment == darkConnectMap.end()) {
                                MapTypeValue ent(*rec);
                                pair<MapType::iterator, bool> ret = darkConnectMap.insert(MapType::value_type(clientAddr, ent));
                                
                                if(ret.second) {
                                    ment = ret.first;
                                } else {
                                    continue;
                                }
                            }

                            ment->second.incConnectionCount();
                            if(ephPort) ment->second.incEphemeralPortCount();
                        }
                    }

                    if(darkConnectMap.size() == 0) {
                        return 0;
                    }

                    //compute the medians
                    int connection_sum = 0;
                    int eph_port_sum = 0;
                    MapType::iterator it = darkConnectMap.begin();
                    while(it != darkConnectMap.end()) {
                        connection_sum += it->second.getConnectionCount();
                        eph_port_sum += it->second.getEphemeralPortCount();
                        
                        it++;
                    }

                    double connMean = (double)connection_sum/(double)darkConnectMap.size();
                    double portMean = (double)eph_port_sum/(double)darkConnectMap.size();

                    double connMedian = 0.0;
                    double portMedian = 0.0;
                    
                    it = darkConnectMap.begin();
                    while(it != darkConnectMap.end()) {
                        double oldDiffConn = abs(connMedian - connMean);
                        double oldDiffPort = abs(portMedian - portMean);
                        double newDiffConn = abs(connMean - it->second.getConnectionCount());
                        double newDiffPort = abs(portMean - it->second.getEphemeralPortCount());

                        if(oldDiffConn > newDiffConn) {
                            connMedian = it->second.getConnectionCount();
                        }
                        
                        if(oldDiffPort > newDiffPort) {
                            portMedian = it->second.getEphemeralPortCount();
                        }

                        it++;
                    }

                    // flag out those above the Median.
                    it = darkConnectMap.begin();
                    while(it != darkConnectMap.end()) {
                        if(it->second.getConnectionCount() > connMedian) {
                            if(!phase2 || it->second.getEphemeralPortCount() > portMedian) {
                                rec = getNetFlowObject(it->second.getFlowEntry());

                                flag(*rec);
                            }
                        }

                        it++;
                    }

                    cout << "# Done." << endl;

                    return 0;
                }

                ~PeerToPeerSymptom() {
                    delete rec1;
                    delete rec2;
                    delete rec3;
                }
            };
        }
    }
}

#endif
