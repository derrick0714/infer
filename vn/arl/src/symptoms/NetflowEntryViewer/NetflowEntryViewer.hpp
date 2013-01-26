#ifndef NETFLOW_VIEWER_HPP
#define NETFLOW_VIEWER_HPP

#include <iostream>
#include <string>

#include "../../shared/arl_parsing/NetFlowRecords.h"
#include "../../shared/TimeStamp.h"
#include "../Symptom.hpp"
#include "../NetflowEntry.h"
#include "NetflowViewerArguments.h"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace std;
            using namespace vn::arl::shared;
            using namespace boost::posix_time;

            template <typename ReaderType>
            class NetflowEntryViewer : public Symptom <NetflowViewerArguments> {
            public:

                explicit NetflowEntryViewer(const NetflowViewerArguments &args, ReaderType &rdr, const string& flag_string, const string& symptomName) :
                  Symptom<NetflowViewerArguments>(args, symptomName, args.getSensorName()),
                  reader(rdr),
                  flagString(flag_string) {
                }

                virtual int run() {
                    const TimeStamp& start = this->s_args.getStartTime();
                    const TimeStamp& end = this->s_args.getEndTime();

                    // this could probably be handled better, but we'll leave that for later.
                    NetFlowARL_v1_Record* rec1 = new NetFlowARL_v1_Record();
                    NetFlowARL_v2_Record* rec2 = new NetFlowARL_v2_Record();
                    NetFlowARL_v3_Record* rec3 = new NetFlowARL_v3_Record();
                    NetFlowARLRecord* rec;

                    // Yes, we are expecting a NetflowEntry type here.
                    // if not... then we are probably screwed.
                    typename ReaderType::value_type entry;

                    while(reader.read(entry)) {
                        switch(entry.getVersion()) {
                            case NetFlowARLRecord::NetFlow_v1:
                                entry.getNetFlow(*rec1);
                                rec = rec1;
                                break;

                            case NetFlowARLRecord::NetFlow_v2:
                                entry.getNetFlow(*rec2);
                                rec = rec2;
                                break;

                            case NetFlowARLRecord::NetFlow_v3:
                                entry.getNetFlow(*rec3);
                                rec = rec3;
                                break;
                        }

                        if(rec->endTime() >= start && rec->endTime() <= end) {
                            this->arl_output(*rec, flagString);
                        }
                    }

                    delete rec1;
                    delete rec2;
                    delete rec3;
                }

            private:
                ReaderType& reader;
                const string flagString;
            };
        }
    }
}

#endif
