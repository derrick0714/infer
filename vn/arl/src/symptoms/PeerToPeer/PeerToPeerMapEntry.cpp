/* 
 * File:   PeerToPeerMapEntry.cpp
 * Author: Mike
 * 
 * Created on January 17, 2010, 2:49 PM
 */

#include "PeerToPeerMapEntry.h"
#include "../../shared/arl_parsing/NetFlowRecords.h"
#include "../../debug/dbg.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    PeerToPeerMapEntry::PeerToPeerMapEntry(const NetFlowARLRecord& netflow) {
		flow = NetflowEntry(netflow);
	    }

	    PeerToPeerMapEntry::PeerToPeerMapEntry(const PeerToPeerMapEntry& orig):
		flow(orig.flow), connectionCount(orig.connectionCount),
                ephemeralPortCount(orig.ephemeralPortCount)
	    {
	    }

	    boost::shared_ptr<Serializable <NetFlowARLRecord> > PeerToPeerMapEntry::getFlow() {
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

            void PeerToPeerMapEntry::incConnectionCount() {
                connectionCount++;
            }

            void PeerToPeerMapEntry::incEphemeralPortCount() {
                ephemeralPortCount++;
            }

            int PeerToPeerMapEntry::getConnectionCount() const {
                return connectionCount;
            }

            int PeerToPeerMapEntry::getEphemeralPortCount() const {
                return ephemeralPortCount;
            }

            NetflowEntry& PeerToPeerMapEntry::getFlowEntry() {
                return flow;
            }

	}
    }
}
