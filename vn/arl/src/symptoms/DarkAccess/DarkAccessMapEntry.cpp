/* 
 * File:   DarkAccessMapEntry.cpp
 * Author: Mike
 * 
 * Created on January 17, 2010, 2:49 PM
 */

#include "DarkAccessMapEntry.h"
#include "../../shared/arl_parsing/NetFlowRecords.h"
#include "../../debug/dbg.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    DarkAccessMapEntry::DarkAccessMapEntry(const NetFlowARLRecord& netflow, const TimeStamp& stamp, const bool isdark):
	    timestamp(stamp),
	    dark(isdark) {
		flow = NetflowEntry(netflow);
	    }

	    DarkAccessMapEntry::DarkAccessMapEntry(const DarkAccessMapEntry& orig):
		timestamp(orig.timestamp),
		dark(orig.dark),
		flow(orig.flow)
	    {
	    }

	    const TimeStamp& DarkAccessMapEntry::getTimeStamp() const {
		return timestamp;
	    }

	    const bool DarkAccessMapEntry::isDark() const {
		return dark;
	    }

	    boost::shared_ptr<Serializable <NetFlowARLRecord> > DarkAccessMapEntry::getFlow() {
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
