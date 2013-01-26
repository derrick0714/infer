/* 
 * File:   NetflowEntry.h
 * Author: Mike
 *
 * Created on January 18, 2010, 12:53 PM
 */

#ifndef _NETFLOWENTRY_H
#define	_NETFLOWENTRY_H

#include "../shared/arl_parsing/NetFlowARLRecord.h"
#include "../shared/DataTypeTraits.hpp"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;

            class NetflowEntry {
            public:
                typedef plain_old_data_tag data_type;

                NetflowEntry();
                NetflowEntry(const NetFlowARLRecord& rec);

                void getNetFlow(NetFlowARLRecord& rec) const;

                NetFlowARLRecord::NetFlowVersion getVersion() const;

            private:
                char data[sizeof(flow_v3_record_t)];
                NetFlowARLRecord::NetFlowVersion version;
            } __attribute__ ((packed));
        }
    }
}

#endif	/* _NETFLOWENTRY_H */

