/* 
 * File:   NetflowEntry.cpp
 * Author: Mike
 * 
 * Created on January 18, 2010, 12:53 PM
 */

#include "../debug/dbg.h"

#include <string>
#include <iostream>

#include "NetflowEntry.h"
#include "../shared/arl_parsing/NetFlowARLRecord.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    using namespace vn::arl::shared;
	    using namespace std;

	    NetflowEntry::NetflowEntry() {
		memset(data, '\0', sizeof(data));
	    }

	    NetflowEntry::NetflowEntry(const NetFlowARLRecord& rec) {
		string sdata;

		rec.serialize(sdata);

		memcpy(data, sdata.data(), sizeof(data));
		version = rec.getVersion();
	    }

	    void NetflowEntry::getNetFlow(NetFlowARLRecord& rec) const {
		string str;

		str.insert(0, data, sizeof(data));
		rec.unserialize(str);
	    }

	    NetFlowARLRecord::NetFlowVersion NetflowEntry::getVersion() const {
		return version;
	    }
	}
    }
}
