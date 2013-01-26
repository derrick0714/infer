#ifndef DNS_FREE_FLOWS_SYMPTOM_HPP
#define DNS_FREE_FLOWS_SYMPTOM_HPP

#include <iostream>
#include <map>

#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../../shared/TimeStamp.h"
#include "../../shared/pmap.hpp"
#include "../../shared/DNS/DNSPacket.hpp"
#include "../../shared/DNS/DNSResponseEntry.h"
#include "../NetflowEntry.h"
#include "../Symptom.hpp"

#include "boost/date_time/posix_time/time_formatters.hpp"
#include "DNSFreeFlowsArguments.h"

// 250MB segments.
#define MAP_SEGMENT_SIZE 250 * 1024 * 1024

namespace vn {
    namespace arl {
        namespace symptom {

	    using namespace std;
	    using namespace vn::arl::shared;
	    using namespace boost::posix_time;

            // Types used for the IP to Flow/flags map
            //typedef pair<address, address> key_type;
            typedef pmap<address, TimeStamp> map_type;

            template <typename NetFlowReaderType, typename DNSReaderType, typename WriterType>
            class DNSFreeFlowsSymptom : public Symptom<DNSFreeFlowsArguments> {
	    private:
		NetFlowReaderType& net_reader;
                DNSReaderType& dns_reader;
                WriterType& writer;

                map_type stateMap;

                bool error;
                string errorString;

                void flag(const NetFlowARLRecord& rec) {
                    // send to writer.
                    writer.write(NetflowEntry(rec));

                    this->arl_output(rec, string("DNS Free Flow symptom"));
                }

            public:
                explicit DNSFreeFlowsSymptom(const DNSFreeFlowsArguments &args, NetFlowReaderType &_net_reader,
                                             DNSReaderType &_dns_reader, WriterType &_writer) :
                        Symptom<DNSFreeFlowsArguments>(args, "DNSFreeFlows", args.getSensorName()),
                        net_reader(_net_reader), 
                        dns_reader(_dns_reader),
                        writer(_writer),
                        error(false) {
                    const boost::filesystem::path& mapFile = args.getStateFile();
                    const char* filename = mapFile.string().c_str();

                    if(!stateMap.create(filename, MAP_SEGMENT_SIZE)) {
                        if(!stateMap.extend(filename, MAP_SEGMENT_SIZE)) {
                            error = true;
                            errorString.assign("Unable to open state");
                        }
                    }
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

                    cout << "# Running DNS Free Flows Symptom Detection." << endl;

                    const int timeout = s_args.getDnsTimeOut();
                    
                    // The last time stamp - tells us what "now" is at the end of scan.
                    TimeStamp lastEndTime;

                    cout << "Scan 1" << endl;
                    // scan 1: enter relevant DNS responses into the map.
                    while (true) {
                        boost::shared_ptr<Serializable <DNSPacket> > recPtr = dns_reader.read();
                        DNSPacket* rec;

                        if (!recPtr) {
                            // we are done with flows.
                            break;
                        }

                        rec = static_cast<DNSPacket*> (&(*recPtr));

                        if(rec->getAnswerCount() > 0) {
                            // Insert the responses into the map.
                            const vector<DNSResponseEntry>& answers = rec->getAnswers();
                            for(vector<DNSResponseEntry>::const_iterator it = answers.begin(); it != answers.end(); ++it) {
                                const DNSResponseEntry& entry = *it;
                                address resolve;

                                if(entry.fillAddress(resolve)) {
                                    // map response addr -> time of resolution
                                    map_type::key_type key(resolve);
                                    TimeStamp value = rec->endTime();

                                    cout << "Entering state" << endl;
                                    stateMap.insert(map_type::value_type(key, value));
                                    cout << "Entered state" << endl;
                                }
                            }
                        }
                    }

                    // scan 2: read the NetFlows and check against DNS entries in the map.
                    cout << "Scan 2" << endl;
                    typedef map<address, NetflowEntry> potentials_type;
                    potentials_type potentials;
                    while (true) {
                        boost::shared_ptr<Serializable <NetFlowARLRecord> > recPtr = net_reader.read();
                        NetFlowARLRecord* rec;

                        if (!recPtr) {
                            // we are done with flows.
                            break;
                        }

                        rec = static_cast<NetFlowARLRecord*> (&(*recPtr));

                        lastEndTime = rec->endTime();

                        if(rec->getClientPort() != 53 && rec->getServerPort() != 53) {
                            map_type::const_iterator dnsEntry = stateMap.find(rec->getServerIP());
                            if(dnsEntry != stateMap.end()) {
                                const TimeStamp& dnsTs = dnsEntry->second;
                                const TimeStamp& tsEndFlow = rec->endTime();

                                int hourDiff = tsEndFlow.seconds() / (60 * 60) - dnsTs.seconds() / (60 * 60);
                                if(hourDiff < 0) {
                                    // the host will be resolved in the future.
                                    //  mean it was probably resolved in the past but overshadowed by future
                                    //  don't flag it.
                                    hourDiff = 0;
                                }

                                if(hourDiff > timeout) {
                                    // resolved too long ago.
                                    flag(*rec);
                                }

                            } else {
                                // maybe it needs to be flagged, but need to check against
                                //  overflowed state files first.
                                cout << "Potential Insert" << endl;
                                potentials.insert(potentials_type::value_type(rec->getServerIP(), NetflowEntry(*rec)));
                                cout << "Potential Inserted" << endl;
                            }
                        }
                    }

                    // scan 2.5
                    cout << "Scan 2.5" << endl;
                    while (stateMap.load_prev_segment() && potentials.size() > 0) {
                        cout << "Next Map" << endl;
                        // start with previous cache and scan the entire map backwards.
                        potentials_type::iterator itr = potentials.begin();
                        while (itr != potentials.end()) {
                            const NetflowEntry& entry = itr->second;

                            NetFlowARLRecord* rec = convertToRecord(entry);

                            map_type::iterator ent = stateMap.find(rec->getServerIP());
                            if (ent != stateMap.end()) {
                                const TimeStamp& dnsTs = ent->second;
                                const TimeStamp& tsEndFlow = rec->endTime();

                                int hourDiff = tsEndFlow.seconds() / (60 * 60) - dnsTs.seconds() / (60 * 60);
                                if(hourDiff < 0) {
                                    // the host will be resolved in the future.
                                    //  mean it was probably resolved in the past but overshadowed by future
                                    //  don't flag it.
                                    hourDiff = 0;
                                }

                                if(hourDiff > timeout) {
                                    // resolved too long ago.
                                    flag(*rec);
                                }

                                potentials.erase(itr);
                            }

                            delete rec;
                            rec = NULL;

                            itr++;
                        }

                        cout << "Next Mapped out" << endl;
                    }

                    cout << "Loop 1" << endl;

                    // those not removed from potentials need to be flagged.
                    potentials_type::const_iterator itr = potentials.begin();
                    while (itr != potentials.end()) {
                        NetFlowARLRecord* rec = convertToRecord(itr->second);
                        
                        flag(*rec);

                        delete rec;
                        rec = NULL;

                        itr++;
                    }

                    // scan 3: try to clean up the state file to makes we don't waste space.
                    // clean, check the oldest map with data. do we still need anything there?
                    while (stateMap.size() == 0 && stateMap.load_next_segment()) {
                        // just move on.
                    }

                    cout << "While 1" << endl;
                    map_type::iterator cls = stateMap.begin();
                    while (cls != stateMap.end()) {
                        const TimeStamp& ts = cls->second;

                        int hourDiff = lastEndTime.seconds() / (60 * 60) - ts.seconds() / (60 * 60);
                        if(hourDiff > timeout) {
                            // Don't need those old entries
                            stateMap.erase(cls);
                        }

                        cls++;
                    }

                    cout << "# Done." << endl;

                    return 0;
                }

                // don't forget to delete the object. This is only a convenience method.
                NetFlowARLRecord* convertToRecord(const NetflowEntry& entry) {
                    NetFlowARLRecord* rec = NULL;

                    switch(entry.getVersion()) {
                        case NetFlowARLRecord::NetFlow_v1:
                            rec = new NetFlowARL_v1_Record();
                            break;

                        case NetFlowARLRecord::NetFlow_v2:
                            rec = new NetFlowARL_v2_Record();
                            break;

                        case NetFlowARLRecord::NetFlow_v3:
                            rec = new NetFlowARL_v3_Record();
                            break;
                    }

                    entry.getNetFlow(*rec);

                    return rec;
                }

            };
        }
    }
}

#endif