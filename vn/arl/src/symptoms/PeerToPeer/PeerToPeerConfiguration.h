/* 
 * File:   PeerToPeerConfiguration.h
 * Author: Mike
 *
 * Created on December 10, 2009, 10:32 PM
 */

#ifndef _PEERTOPEERCONFIGURATION_H
#define	_PEERTOPEERCONFIGURATION_H

#include <string>
#include <boost/filesystem.hpp>

#include "../IPZone.h"
#include "../../shared/SampleSynappConfiguration.h"

#define DEFAULT_TW_VALUE 24
#define DEFAULT_STATE_FILE "./"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;
            using namespace std;

            class PeerToPeerConfiguration : public SynappConfiguration {
            public:
                /// \brief Constructor for parsing PeerToPeer config file.
                /// \param filename Configuration file path.
                ///
                /// Constructs the object and fills in the values from a file.
                PeerToPeerConfiguration(const boost::filesystem::path &fileName);

                /// \returns the strings that resprested the subnet being
                ///          monitored.
                const IPZone& getMonitoredZone() const;

                int getEphemeralPortStart() const;

                int getEphemeralPortEnd() const;

                bool isPhaseTwo() const;
               
            private:
                /// \brief sets the options to be parsed out the file.
                virtual void setOptionsDescriptions();

                /// \brief performs the actual parsing.
                virtual void parseOptions();

                IPZone zone;

                int _ephPortStart;

                int _ephPortEnd;

                bool isPhase2;
            };

        }
    }
}

#endif	/* _PEERTOPEERCONFIGURATION_H */

