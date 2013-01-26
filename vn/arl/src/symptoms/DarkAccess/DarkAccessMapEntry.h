/* 
 * File:   DarkAccessMapEntry.h
 * Author: Mike
 *
 * Created on January 17, 2010, 2:49 PM
 */

#ifndef _DARKACCESSMAPENTRY_H
#define	_DARKACCESSMAPENTRY_H

#include <boost/shared_ptr.hpp>
#include "../../shared/TimeStamp.h"
#include "../../shared/arl_parsing/NetFlowARLRecord.h"
#include "../NetflowEntry.h"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;

            class DarkAccessMapEntry {
            public:
                explicit DarkAccessMapEntry(const NetFlowARLRecord& netflow, const TimeStamp& stamp, const bool isdark);

                DarkAccessMapEntry(const DarkAccessMapEntry& orig);

                const TimeStamp& getTimeStamp() const;

                const bool isDark() const;

                boost::shared_ptr<Serializable <NetFlowARLRecord> > getFlow();
            private:

                /// \brief we save the offending flow in a DarkAccessEntry class
                ///        for easy retrieval.
                NetflowEntry flow;

                /// \brief Time stamp of when the offense happened, saved off separately from
                ///        the flow because we'll access this often and we don't want to spend
                ///        time deserializing the flow everytime.
                TimeStamp timestamp;

                /// \brief Was the server (the key in the map) marked as "been access from outside"
                bool dark;
            } __attribute__ ((packed));

        }
    }
}

#endif	/* _DARKACCESSMAPENTRY_H */

