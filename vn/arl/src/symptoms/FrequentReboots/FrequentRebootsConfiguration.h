/* 
 * File:   FrequentRebootsConfiguration.h
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

#define DEFAULT_STATE_DIR "./"
#define DEFAULT_APP_FILE "/usr/local/etc/boot_applications.conf"
#define DEFAULT_EVENTS_AT_REBOOT 3
#define DEFAULT_BOOT_TIME_WINDOW 300
#define DEFAULT_WATCH_TIME 24
#define DEFAULT_REBOOT_COUNT 5

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;
            using namespace std;

            class FrequentRebootsConfiguration : public SynappConfiguration {
            public:
                /// \brief Constructor for parsing FrequentReboots config file.
                /// \param filename Configuration file path.
                ///
                /// Constructs the object and fills in the values from a file.
                FrequentRebootsConfiguration(const boost::filesystem::path &fileName);

                const boost::filesystem::path& getApplicationDefinitionFile() const;

                const boost::filesystem::path& getStateDirectory() const;

                int getEventsAtReboot() const;

                int getBootTimeWindow() const;

                int getRebootCount() const;

                int getWatchTime() const;
                
            private:
                /// \brief sets the options to be parsed out the file.
                virtual void setOptionsDescriptions();

                /// \brief performs the actual parsing.
                virtual void parseOptions();

                /// \brief Application definition file.
                boost::filesystem::path appDef;

                /// \brief state directory. For storing maps.
                boost::filesystem::path stateDir;

                /// \brief How many events constitute a reboot
                int eventsAtReboot;

                /// \brief At what time window should the events happen to declare
                ///        a reboot
                int bootTimeWindow;

                /// \brief How many reboots should there be before we flag the
                ///        host
                int rebootCount;

                /// \brief At what time window should the rebootCount events
                ///        happen
                int watchTime;
            };

        }
    }
}

#endif	/* _DARKACCESSCONFIGURATION_H */

