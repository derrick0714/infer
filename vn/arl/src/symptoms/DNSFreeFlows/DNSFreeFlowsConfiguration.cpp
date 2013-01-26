/* 
 * File:   EvasiveTrafficConfiguration.cpp
 * Author: Mike
 * 
 * Created on December 10, 2009, 10:32 PM
 */

#include "DNSFreeFlowsConfiguration.h"
#include "DNSFreeFlowsParams.h"

namespace vn {
    namespace arl {
	namespace symptom {
	    DNSFreeFlowsConfiguration::DNSFreeFlowsConfiguration(const boost::filesystem::path &fileName):
	    SynappConfiguration(fileName) {
		setOptionsDescriptions();
	    	parseOptions();
	    }

	    void DNSFreeFlowsConfiguration::setOptionsDescriptions() {
		options.add_options()
			(
			CFG_DNS_TIMEOUT,
			boost::program_options::value <int> (),
			"DNS Timeout value"
			)
			(
			CFG_STATE_FILE,
			boost::program_options::value <int> (),
			"State files"
			)
			;
	    }

	    void DNSFreeFlowsConfiguration::parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
		    return;
		}

		if (vals.count(CFG_DNS_TIMEOUT)) {
		    timeout = vals[CFG_DNS_TIMEOUT].as<int> ();
		}

                if (vals.count(CFG_STATE_FILE)) {
		    state = boost::filesystem::path(vals[CFG_STATE_FILE].as<std::string> ());
		}
	    }

            const boost::filesystem::path& DNSFreeFlowsConfiguration::getStateFile() const {
		return state;
	    }

	    const int DNSFreeFlowsConfiguration::getTimeOut() const {
		return timeout;
	    }

	}
    }
}