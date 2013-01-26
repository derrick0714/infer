/* 
 * File:   DarkAccessConfiguration.cpp
 * Author: Mike
 * 
 * Created on December 10, 2009, 10:32 PM
 */

#include <iostream>
#include "DarkAccessConfiguration.h"
#include "DarkAccessParams.h"
#include "../IPsubnet.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    DarkAccessConfiguration::DarkAccessConfiguration(const boost::filesystem::path &fileName) :
	    SynappConfiguration(fileName),
	    hours(DEFAULT_TW_VALUE),
	    state(DEFAULT_STATE_FILE) {
		setOptionsDescriptions();
		parseOptions();
	    }

	    const int DarkAccessConfiguration::getTimeWindow() const {
		return hours;
	    }

	    const IPZone DarkAccessConfiguration::getMonitoredZone() const {
		return zone;
	    }

	    const boost::filesystem::path& DarkAccessConfiguration::getStateFile() const {
		return state;
	    }

	    void DarkAccessConfiguration::setOptionsDescriptions() {
		options.add_options()
			(
			CFG_TW_VALUE,
			boost::program_options::value <int> (),
			"Time window value in hours. 24 is Default"
			)
			(
			CFG_ZONE_VALUE,
			boost::program_options::value<string > (),
			"Monitored network zone."
			);
	    }

	    void DarkAccessConfiguration::parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
		    return;
		}

		if (vals.count(CFG_TW_VALUE)) {
		    hours = vals[CFG_TW_VALUE].as<int> ();
		}

		if (vals.count(CFG_STATE_FILE)) {
		    state = boost::filesystem::path(vals[CFG_STATE_FILE].as<string > ());
		}

		if (vals.count(CFG_ZONE_VALUE)) {
		    string zone_str = vals[CFG_ZONE_VALUE].as<string > ();

		    IPsubnet sn = zone.addAddressRangeList(zone_str);

		    if (sn.errored()) {
			std::cout << "Error parsing zone string: " << zone_str << std::endl;
		    }
		}
	    }
	}
    }
}
