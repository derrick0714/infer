#ifndef EVASIVE_TRAFFIC_HPP
#define EVASIVE_TRAFFIC_HPP

#include <iostream>

#include "../../debug/dbg.h"
#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../../shared/TimeStamp.h"
#include "boost/date_time/posix_time/time_formatters.hpp"

#include "../Symptom.hpp"
#include "EvasiveTrafficArguments.h"
#include "../NetflowEntry.h"

namespace vn {
    namespace arl {
        namespace symptom {

	    using namespace std;
	    using namespace vn::arl::shared;
	    using namespace boost::posix_time;

            template <typename ReaderType, typename WriterType>
            class EvasiveTrafficSymptom : public Symptom <EvasiveTrafficArguments> {
            public:
                explicit EvasiveTrafficSymptom(const EvasiveTrafficArguments &args, ReaderType &reader, WriterType &writer);

                virtual int run();

	    private:
                void flag(NetFlowARLRecord& rec);

		ReaderType& reader;
                WriterType& writer;
            };

	    template <typename ReaderType, typename WriterType>
	    EvasiveTrafficSymptom<ReaderType, WriterType>::EvasiveTrafficSymptom(const EvasiveTrafficArguments &args, ReaderType &rdr, WriterType &wtr):
	    Symptom<EvasiveTrafficArguments>(args, "EvasiveTraffic", args.getSensorName()), reader(rdr), writer(wtr)
	    {
	    }

	    template <typename ReaderType, typename WriterType>
            int EvasiveTrafficSymptom<ReaderType, WriterType>::run() {
                using namespace std;

                DBG(cout << "# Debug mode, flagging everything" << endl);
                cout << "# Running Evasive Traffic Symptom Detection." << endl;

                int ttlval = s_args.getTTL();
		NetFlowARLRecord* rec;

		while(true) {
		    boost::shared_ptr<Serializable <NetFlowARLRecord> > recPtr;

		    recPtr = reader.read();
		    if(!recPtr) {
			// we are done with flows.
			break;
		    }

		    rec = static_cast<NetFlowARLRecord*>(&(*recPtr));

                    // Do the actual checks.
                    //  Logic: TTL < t-value or Don't fragment or More fragments.
                    if(rec->getTtl() < ttlval) {
                        flag(*rec);
                    } else {
                        //this step requires NetFlow V3
                        if(rec->getVersion() == NetFlowARLRecord::NetFlow_v3) {
                            const session_flags_t* sflags = rec->getSessionFlags();
                            uint8_t ipFlags = sflags->misc_flags.ip_flags;

                            if(ipFlags & IP_FLAGS_DP || ipFlags & IP_FLAGS_MP) {
                                flag(*rec);
                            }
                        }
                    }

                    DBG(flag(*rec));
		}

                cout << "# Done." << endl;

                return 0;
            }

            template <typename ReaderType, typename WriterType>
            void EvasiveTrafficSymptom<ReaderType, WriterType>::flag(NetFlowARLRecord& rec) {
                // send to writer.
                writer.write(NetflowEntry(rec));

                this->arl_output(rec, string("Evasive traffic symptom"));
            }
        }
    }
}

#endif
