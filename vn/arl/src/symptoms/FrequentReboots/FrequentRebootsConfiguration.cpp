/* 
 * File:   FrequentRebootsConfiguration.cpp
 * Author: Mike
 * 
 * Created on December 10, 2009, 10:32 PM
 */

#include <iostream>
#include "FrequentRebootsConfiguration.h"
#include "FrequentRebootsParams.h"
#include "../IPsubnet.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    FrequentRebootsConfiguration::FrequentRebootsConfiguration(const boost::filesystem::path &fileName) :
                    SynappConfiguration(fileName),
                    stateDir(DEFAULT_STATE_DIR),
                    appDef(DEFAULT_APP_FILE),
                    eventsAtReboot(DEFAULT_EVENTS_AT_REBOOT),
                    bootTimeWindow(DEFAULT_BOOT_TIME_WINDOW),
                    rebootCount(DEFAULT_REBOOT_COUNT),
                    watchTime(DEFAULT_WATCH_TIME)
	    {
		setOptionsDescriptions();
		parseOptions();
	    }

            const boost::filesystem::path& FrequentRebootsConfiguration::getApplicationDefinitionFile() const {
                return appDef;
            }

            const boost::filesystem::path& FrequentRebootsConfiguration::getStateDirectory() const {
                return stateDir;
            }
            
            int FrequentRebootsConfiguration::getEventsAtReboot() const {
                return eventsAtReboot;
            }

            int FrequentRebootsConfiguration::getBootTimeWindow() const {
                return bootTimeWindow;
            }

            int FrequentRebootsConfiguration::getRebootCount() const {
                return rebootCount;
            }

            int FrequentRebootsConfiguration::getWatchTime() const {
                return watchTime;
            }

            void FrequentRebootsConfiguration::setOptionsDescriptions() {
		options.add_options()
                        (
                        CFG_APP_DEF,
                        boost::program_options::value<std::string> (),
                        "Application definition file"
                        )
			(
			CFG_STATE_DIR,
			boost::program_options::value <std::string> (),
			"Directory for storing state files."
			)
                        (
                        CFG_EVENTS_AT_REBOOT,
                        boost::program_options::value <int> (),
			"How many events constitute a reboot."
			)
                        (
                        CFG_BOOT_TIME_WINDOW,
                        boost::program_options::value <int> (),
			"At what time window should the events happen to declare a reboot."
			)
                        (
                        CFG_WATCH_TIME,
                        boost::program_options::value <int> (),
			"How many reboots should there be before we flag the host."
			)
                        (
                        CFG_REBOOT_COUNT,
                        boost::program_options::value <int> (),
			"At what time window should the rebootCount events happen."
			);
	    }

	    void FrequentRebootsConfiguration::parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
		    return;
		}

		if (vals.count(CFG_APP_DEF)) {
		    appDef = boost::filesystem::path(vals[CFG_APP_DEF].as<std::string> ());
		}

		if (vals.count(CFG_STATE_DIR)) {
                    stateDir = boost::filesystem::path(vals[CFG_STATE_DIR].as<std::string > ());
		}

                if (vals.count(CFG_EVENTS_AT_REBOOT)) {
		    eventsAtReboot = vals[CFG_EVENTS_AT_REBOOT].as<int> ();
		}

                if (vals.count(CFG_BOOT_TIME_WINDOW)) {
		    bootTimeWindow = vals[CFG_BOOT_TIME_WINDOW].as<int> ();
		}

                if (vals.count(CFG_WATCH_TIME)) {
		    watchTime = vals[CFG_WATCH_TIME].as<int> ();
		}

                if (vals.count(CFG_REBOOT_COUNT)) {
		    rebootCount = vals[CFG_REBOOT_COUNT].as<int> ();
		}
	    }
	}
    }
}
