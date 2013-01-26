/* 
 * File:   FrequentRebootsMapEntry.cpp
 * Author: Mike
 * 
 * Created on January 17, 2010, 2:49 PM
 */

#include "FrequentRebootsMapEntry.h"
#include "../../shared/arl_parsing/NetFlowRecords.h"
#include "../../debug/dbg.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    FrequentRebootsMapEntry::FrequentRebootsMapEntry(const NetFlowARLRecord& netflow, const TimeStamp& stamp, const bool isdark):
	    timestamp(stamp),
	    dark(isdark) {
		flow = NetflowEntry(netflow);
	    }

	    FrequentRebootsMapEntry::FrequentRebootsMapEntry(const FrequentRebootsMapEntry& orig):
		timestamp(orig.timestamp),
		dark(orig.dark),
		flow(orig.flow)
	    {
	    }

	    const TimeStamp& FrequentRebootsMapEntry::getTimeStamp() const {
		return timestamp;
	    }

	    const bool FrequentRebootsMapEntry::isDark() const {
		return dark;
	    }

	    boost::shared_ptr<Serializable <NetFlowARLRecord> > FrequentRebootsMapEntry::getFlow() {
		NetFlowARLRecord* rec = NULL;

		switch(flow.getVersion()) {
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

		flow.getNetFlow(*rec);

		return boost::shared_ptr<Serializable <NetFlowARLRecord> >(rec);
	    }

	}
    }
}
