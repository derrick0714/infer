#ifndef DARK_ACCESS_HPP
#define DARK_ACCESS_HPP

#include "../../debug/dbg.h"

#include <iostream>
#include <set>
#include <map>

#include "../../shared/SingleFileEnumerator.hpp"
#include "../../shared/DNS/DnsPcapReader.hpp"
#include "../../shared/DNS/DNSPacket.hpp"
#include "../../shared/DNS/DnsPcapReader.hpp"
#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../../shared/TimeStamp.h"
#include "../../shared/pmultimap.hpp"

#include "../Symptom.hpp"
#include "../NetflowEntry.h"
#include "FrequentRebootsArguments.h"
#include "FrequentRebootsMapEntry.h"

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
            //  for first map (app_map) we really just need to know the time of an HEI Event
            // Holds Client IP mapping to Time of HIE
            typedef pmultimap<address, TimeStamp> app_map_type;
            typedef app_map_type::range app_range_type;

            // holds Client IP mapping to Time of reboot.
            typedef pmultimap<address, TimeStamp> reboot_map_type;
            typedef reboot_map_type::range reboot_range_type;

            template <typename ReaderType, typename WriterType>
            class FrequentRebootsSymptom : public Symptom<FrequentRebootsArguments> {
            private:

                ReaderType& reader;
                WriterType& writer;
                app_map_type app_map;
                reboot_map_type reboot_map;

                bool error;
                string errorString;

                void flag(NetFlowARLRecord& rec) {
                    // send to writer.
                    writer.write(NetflowEntry(rec));

                    this->arl_output(rec, string("Client frequent reboot detected"));
                }

            public:

                explicit FrequentRebootsSymptom(const FrequentRebootsArguments &args, ReaderType &rdr, WriterType &wtr) :
                Symptom<FrequentRebootsArguments>(args, "FrequentReboots", args.getSensorName()),
                reader(rdr), writer(wtr), error(false) {
                    const boost::filesystem::path& app_map_file = args.getHostAppMapFile();
                    const char* filename = app_map_file.string().c_str();

                    if(!app_map.create(filename, MAP_SEGMENT_SIZE)) {
                        if(!app_map.extend(filename, MAP_SEGMENT_SIZE)) {
                            error = true;
                            errorString.assign("Unable to open app map file");
                        }
                    }

                    const boost::filesystem::path& reboot_map_file = args.getHostRebootFile();
                    filename = reboot_map_file.string().c_str();

                    if(!reboot_map.create(filename, MAP_SEGMENT_SIZE)) {
                        if(!reboot_map.extend(filename, MAP_SEGMENT_SIZE)) {
                            error = true;
                            errorString.assign("Unable to open reboot map file");
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

                    cout << "# Running Frequent Reboots Symptom Detection." << endl;

                    // What apps are we looking for?
                    HIEAppStore& store = this->s_args.getApplicationStore();

                    // go through DNS resolutions.
                    SingleFileEnumerator fenum_pcap(s_args.inputPcapFile());
                    DnsPcapReader<SingleFileEnumerator> dns_reader(fenum_pcap);
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
                                string name;
                                address resolve;

                                if(entry.fillAddress(resolve)) {
                                    name = rec->getDnsName(entry.getLabels());

                                    store.storeDNSMapping(name, resolve);
                                }
                            }
                        }
                    }

                    // The last time stamp - tells us what "now" is at the end of scan.
                    TimeStamp lastEndTime;
                    TimeStamp lastStartTime;

                    int events_at_reboot = this->s_args.getEventsAtReboot();
                    uint32_t boot_time_window = this->s_args.getBootTimeWindow(); //seconds

                    int reboots_count = this->s_args.getRebootCount();
                    uint32_t watch_window = this->s_args.getWatchTime() * 60 * 60; //seconds

                    // Scan 1: mark hosts that have rebooted.
                    while (true) {
                        boost::shared_ptr<Serializable <NetFlowARLRecord> > recPtr = reader.read();
                        NetFlowARLRecord* rec;

                        if (!recPtr) {
                            // we are done with flows.
                            break;
                        }

                        rec = static_cast<NetFlowARLRecord*> (&(*recPtr));

                        lastStartTime = rec->startTime();
                        lastEndTime = rec->endTime();
                        const address clientIP = rec->getClientIP();

                        HIEApplication app(rec->startTime(), rec->getServerIP(), rec->getServerPort());

                        if(store.contains(app)) {
                            // If here then app is an HIE, remember when it happened.
                            app_map.insert(app_map_type::value_type(clientIP, app.getTimeStamp()));

                            int passCount = 0;
                            int eventCount = 0;
                            bool markReboot = false;
                            bool pastWindow = false;

                            do {
                                // This loop: Checking if the host has rebooted.
                                passCount++;

                                // this should include the thing we added above.
                                app_range_type hieSet = app_map.equal_range(clientIP);
                                for(app_map_type::iterator it = hieSet.first; it != hieSet.second;) {
                                    bool doInc = true;
                                    // check for reboot, clean up and mark if reboot.
                                    const TimeStamp& check_stamp = it->second;

                                    uint32_t timeDiff = (lastStartTime - check_stamp).seconds();
                                    if(timeDiff <= boot_time_window) {
                                        // if the HIE is within the timeframe then count it in.
                                        eventCount++;

                                        // reboot happened.
                                        if(eventCount >= events_at_reboot) {
                                            markReboot = true;

                                            break;
                                        }
                                    } else {
                                        pastWindow = true;
                                        doInc = false;
                                        app_map.erase(it++);
                                    }

                                    if(doInc) {
                                        ++it;
                                    }
                                }
                                // Only if it hasn't been marked as reboot
                                // expected to not happen often.
                            } while (app_map.load_prev_segment() && !pastWindow && !markReboot);

                            if(passCount > 1) {
                                // next time we'll need to be in the latest;
                                app_map.load_latest_segment();
                            }

                            if(markReboot) {
                                // Mark reboot
                                reboot_map.insert(reboot_map_type::value_type(clientIP, app.getTimeStamp()));

                                // clean up, we don't need anything for this host in app_map.
                                app_map.erase(clientIP);

                                // Scan 2: mark hosts that frequenty reboot.
                                int loopCount = 0;
                                int rebootCount = 0;
                                bool do_flag = false;
                                bool outside_window = false;

                                do {
                                    loopCount++;
                                    
                                    reboot_range_type reboots = reboot_map.equal_range(clientIP);
                                    for(reboot_map_type::iterator it = reboots.first; it != reboots.second;) {
                                        bool doInc = true;
                                        const TimeStamp& check_time = it->second;

                                        uint32_t timeDiff = (lastStartTime - check_time).seconds();
                                        if(timeDiff <= watch_window) {
                                            rebootCount++;

                                            if(rebootCount >= reboots_count) {
                                                do_flag = true;

                                                break;
                                            }
                                        } else {
                                            // outside of the window. get rid of it.
                                            outside_window = true;
                                            doInc = false;
                                            reboot_map.erase(it++);
                                        }

                                        if(doInc) {
                                            ++it;
                                        }
                                    }
                                } while (reboot_map.load_prev_segment() && !do_flag && !outside_window);

                                if(loopCount > 1) {
                                    reboot_map.load_latest_segment();
                                }

                                // report a frequent reboot.
                                if(do_flag) {
                                    flag(*rec);

                                    // clean up. we don't need stuff associated with this host
                                    reboot_map.erase(clientIP);
                                }
                            }
                        }
                    }

                    // here we'll need to do clean up on reboot_map and app_map.
                    reboot_map.load_first_segment();
                    reboot_map_type::iterator rmap = reboot_map.begin();
                    while(rmap != reboot_map.end()) {
                        const TimeStamp& ts = rmap->second;

                        TimeStamp diff = lastStartTime - ts;
                        if(diff.seconds() > watch_window) {
                            reboot_map.erase(rmap++);
                        } else {
                            ++rmap;
                        }
                    }

                    app_map.load_first_segment();
                    app_map_type::iterator amap = app_map.begin();
                    while(amap != app_map.end()) {
                        const TimeStamp& ts = amap->second;

                        TimeStamp diff = lastStartTime - ts;
                        if(diff.seconds() > boot_time_window) {
                            app_map.erase(amap++);
                        } else {
                            ++amap;
                        }
                    }

                    cout << "# Done." << endl;

                    return 0;
                }

                ~FrequentRebootsSymptom() {
                    app_map.close();
                    reboot_map.close();
                }
            };
        }
    }
}

#endif
