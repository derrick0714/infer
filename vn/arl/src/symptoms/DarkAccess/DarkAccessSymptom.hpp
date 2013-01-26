#ifndef DARK_ACCESS_HPP
#define DARK_ACCESS_HPP

#include "../../debug/dbg.h"

#include <iostream>
#include <set>

#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../../shared/TimeStamp.h"
#include "../../shared/pmap.hpp"

#include "../Symptom.hpp"
#include "../NetflowEntry.h"
#include "DarkAccessArguments.h"
#include "DarkAccessMapEntry.h"

// 250MB segments.
#define MAP_SEGMENT_SIZE 250 * 1024 * 1024

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace std;
            using namespace vn::arl::shared;
            using namespace boost::posix_time;
            using namespace boost::asio::ip;

            // Types used for the IP to Flow/flags map
            typedef pmap<address, DarkAccessMapEntry> map_type;
            typedef pair<address, DarkAccessMapEntry> pair_type;

            template <typename ReaderType, typename WriterType>
            class DarkAccessSymptom : public Symptom<DarkAccessArguments> {
            private:

                ReaderType& reader;
                WriterType& writer;
                map_type stateMap;

                bool error;
                string errorString;

                void flag(NetFlowARLRecord& rec) {
                    // send to writer.
                    writer.write(NetflowEntry(rec));

                    this->arl_output(rec, string("Dark space access symptom"));
                }

            public:

                explicit DarkAccessSymptom(const DarkAccessArguments &args, ReaderType &rdr, WriterType &wtr) :
                Symptom<DarkAccessArguments>(args, "DarkAccess", args.getSensorName()),
                reader(rdr), writer(wtr), error(false) {
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

                    cout << "# Running Dark Access Symptom Detection." << endl;

                    // this set represents servers that are potentially dark accessed.
                    //  it used to report server after all netflows have been scanned.
                    set<address> potentials;

                    // The last time stamp - tells us what "now" is at the end of scan.
                    TimeStamp lastEndTime;

                    const IPZone& zone = this->s_args.getMonitoredZone();
                    const unsigned int timeInterval = this->s_args.getTimeWindow() * 60 * 60; // convert to seconds.

                    DBG(cout << "Dark access, starting loop 1" << endl);

                    // 1st scan. loop through all the flows and map flow that could potentially be flagged
                    // 2nd scan will reconfirm with older data.
                    while (true) {
                        boost::shared_ptr<Serializable <NetFlowARLRecord> > recPtr = reader.read();
                        NetFlowARLRecord* rec;

                        if (!recPtr) {
                            // we are done with flows.
                            break;
                        }

                        rec = static_cast<NetFlowARLRecord*> (&(*recPtr));

                        const address& serverIP = rec->getServerIP();
                        const address& clientIP = rec->getClientIP();
                        const TimeStamp& endTime = rec->endTime();

                        lastEndTime = endTime;

                        //DBG(cout << "Server: " << serverIP.to_string() << " Client: " << clientIP.to_string() << endl);

                        if (zone.inRange(clientIP) && !zone.inRange(serverIP)) {
                            // connection out;
                            //  this tells us not to mark the server with dark access because of this
                            //  connection.
                            stateMap.insert(pair_type(serverIP, DarkAccessMapEntry(*rec, endTime, false)));
                        } else if (!zone.inRange(clientIP) && zone.inRange(serverIP)) {
                            // connection in;
                            map_type::const_iterator entry = stateMap.find(serverIP); //yes, I just want to check the cache

                            if (entry == stateMap.end()) {
                                // entry not found so we need to record as a potential dark access
                                stateMap.insert(pair_type(serverIP, DarkAccessMapEntry(*rec, endTime, true)));
                                potentials.insert(serverIP);
                            } else if (!entry->second.isDark()) {
                                uint32_t secondsDiff = (endTime - entry->second.getTimeStamp()).seconds();

                                // when the server has been marked not dark we need to make sure it did make
                                //  a connection out within the allowed time interval.
                                if (secondsDiff > timeInterval) {
                                    stateMap.insert(pair_type(serverIP, DarkAccessMapEntry(*rec, endTime, true)));
                                    potentials.insert(serverIP);
                                }
                            }
                        }
                    }

                    DBG(cout << "Dark access, starting loop 2" << endl);

                    // 2nd scan. here we have a list of all potential servers that could've been
                    // dark accessed, now we have to verify that list through all the segments.
                    do {
                        // start with current cache and scan the entire map backwards.
                        set<address>::const_iterator itr = potentials.begin();
                        while (itr != potentials.end()) {
                            const address& addr = *itr;

                            map_type::iterator ent = stateMap.find(addr);
                            if (ent != stateMap.end()) {
                                const DarkAccessMapEntry& entry = ent->second;

                                // Does this mark the entry as not dark?
                                if (!entry.isDark()) {
                                    // now we need to find out if it called out in the allowable time.
                                    uint32_t secondsDiff = (lastEndTime - entry.getTimeStamp()).seconds();

                                    if (secondsDiff > timeInterval) {
                                        // entry too old, we don't care about it anymore.
                                        stateMap.erase(ent);
                                    } else {
                                        // No, this server is not dark. take it out of the set.
                                        potentials.erase(itr);
                                    }
                                }
                            }

                            itr++;
                        }
                    } while (stateMap.load_prev_segment());

                    // this needs to be improved. move to lasted segment
                    if (!stateMap.load_latest_segment()) {
                        error = true;
                        errorString.append("Error to read the state file");

                        return 1;
                    }

                    DBG(cout << "Dark access, starting loop 3" << endl);

                    // 3nd scan. we now know what servers are dark accessed. now let's find which
                    // flows caused it. This will potentially flag the same servers every hour.
                    do {
                        if (stateMap.size() > 0) {
                            set<address>::const_iterator itr = potentials.begin();
                            while (itr != potentials.end()) {
                                const address& addr = *itr;
                                map_type::iterator ent = stateMap.find(addr);

                                if (ent != stateMap.end()) {
                                    DarkAccessMapEntry& entry = ent->second;
                                    boost::shared_ptr<Serializable <NetFlowARLRecord> > recPtr = entry.getFlow();
                                    NetFlowARLRecord* rec = static_cast<NetFlowARLRecord*> (&(*recPtr));

                                    // since we are going backwards, we know this is the lasted record.
                                    flag(*rec);

                                    // ok we flagged it, don't care about it anymore.
                                    potentials.erase(itr);
                                }

                                itr++;
                            }
                        }
                    } while (stateMap.load_prev_segment() && potentials.size() > 0);

                    // clean, check the oldest map with data. do we still need anything there?
                    while (stateMap.size() == 0 && stateMap.load_next_segment()) {
                        // just move on.
                    }

                    map_type::iterator itr = stateMap.begin();
                    while (itr != stateMap.end()) {
                        DarkAccessMapEntry* entry = &(itr->second);
                        uint32_t secondsDiff = (lastEndTime - entry->getTimeStamp()).seconds();

                        if (secondsDiff > timeInterval) {
                            // entry too old, we don't care about it anymore.
                            stateMap.erase(itr);
                        }
                        itr++;
                    }

                    cout << "# Done." << endl;

                    return 0;
                }

                ~DarkAccessSymptom() {
                    stateMap.close();
                }
            };
        }
    }
}

#endif
