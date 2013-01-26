/* 
 * File:   DarkAccessConfiguration.h
 * Author: Mike
 *
 * Created on December 10, 2009, 10:32 PM
 */

#ifndef _DARKACCESSCONFIGURATION_H
#define	_DARKACCESSCONFIGURATION_H

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

            class DarkAccessConfiguration : public SynappConfiguration {
            public:
                /// \brief Constructor for parsing DarkAccess config file.
                /// \param filename Configuration file path.
                ///
                /// Constructs the object and fills in the values from a file.
                DarkAccessConfiguration(const boost::filesystem::path &fileName);

                /// \returns the time window in terms of hours.
                const int getTimeWindow() const;

                /// \returns the strings that resprested the subnet being
                ///          monitored.
                const IPZone getMonitoredZone() const;

                const boost::filesystem::path& getStateFile() const;
                
            private:
                /// \brief sets the options to be parsed out the file.
                virtual void setOptionsDescriptions();

                /// \brief performs the actual parsing.
                virtual void parseOptions();

                int hours;

                IPZone zone;

                boost::filesystem::path state;
            };

        }
    }
}

#endif	/* _DARKACCESSCONFIGURATION_H */

