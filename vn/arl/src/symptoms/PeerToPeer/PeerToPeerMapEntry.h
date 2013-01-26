/* 
 * File:   PeerToPeerMapEntry.h
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

            class PeerToPeerMapEntry {
            public:
                explicit PeerToPeerMapEntry(const NetFlowARLRecord& netflow);

                PeerToPeerMapEntry(const PeerToPeerMapEntry& orig);

                boost::shared_ptr<Serializable <NetFlowARLRecord> > getFlow();

                NetflowEntry& getFlowEntry();

                void incConnectionCount();

                void incEphemeralPortCount();

                int getConnectionCount() const;

                int getEphemeralPortCount() const;
            private:

                /// \brief we save the offending flow in a PeerToPeerEntry class
                ///        for easy retrieval.
                NetflowEntry flow;
                int connectionCount;
                int ephemeralPortCount;
            } __attribute__ ((packed));

        }
    }
}

#endif	/* _DARKACCESSMAPENTRY_H */

