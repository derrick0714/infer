/* 
 * File:   EvasiveTrafficConfiguration.cpp
 * Author: Mike
 * 
 * Created on December 10, 2009, 10:32 PM
 */

#include "EvasiveTrafficConfiguration.h"
#include "EvasiveTrafficParams.h"

namespace vn {
    namespace arl {
	namespace symptom {
	    EvasiveTrafficConfiguration::EvasiveTrafficConfiguration(const boost::filesystem::path &fileName):
	    SynappConfiguration(fileName),
	    ttl(DEFAULT_TTL_VALUE){
		setOptionsDescriptions();
	    	parseOptions();
	    }

	    const int EvasiveTrafficConfiguration::getTTL() const {
		return ttl;
	    }

	    void EvasiveTrafficConfiguration::setOptionsDescriptions() {
		options.add_options()
			(
			CFG_TTL_VALUE,
			boost::program_options::value <int> (),
			"TTL Threshold value"
			)
			;
	    }

	    void EvasiveTrafficConfiguration::parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
		    return;
		}

		if (vals.count(CFG_TTL_VALUE)) {
		    ttl = vals[CFG_TTL_VALUE].as<int> ();
		}
	    }
	}
    }
}