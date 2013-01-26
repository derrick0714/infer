/* 
 * File:   PeerToPeerConfiguration.cpp
 * Author: Mike
 * 
 * Created on December 10, 2009, 10:32 PM
 */

#include <boost/lexical_cast.hpp>
#include <iostream>
#include "PeerToPeerConfiguration.h"
#include "PeerToPeerParams.h"
#include "../IPsubnet.h"

namespace vn {
    namespace arl {
	namespace symptom {

	    PeerToPeerConfiguration::PeerToPeerConfiguration(const boost::filesystem::path &fileName) :
	    SynappConfiguration(fileName), _ephPortStart(1024), _ephPortEnd(65535), isPhase2(true) {
		setOptionsDescriptions();
		parseOptions();
	    }

	    const IPZone& PeerToPeerConfiguration::getMonitoredZone() const {
		return zone;
	    }

            int PeerToPeerConfiguration::getEphemeralPortStart() const {
                return _ephPortStart;
            }

            int PeerToPeerConfiguration::getEphemeralPortEnd() const {
                return _ephPortEnd;
            }

            bool PeerToPeerConfiguration::isPhaseTwo() const {
                return isPhase2;
            }

	    void PeerToPeerConfiguration::setOptionsDescriptions() {
		options.add_options()
			(
			CFG_ZONE_VALUE,
			boost::program_options::value<string > (),
			"Monitored network zone."
			)
                        (
			CFG_PHASE2,
			boost::program_options::value<string > (),
			"Perform phase 2 processing. True/False."
			)
                        (
			CFG_EPH_PORT,
			boost::program_options::value<string > (),
			"Ephemeral port definition (1024-65535)."
			);
	    }

	    void PeerToPeerConfiguration::parseOptions() {
		boost::program_options::variables_map vals;
		if (!_parseOptions(vals)) {
		    return;
		}

		if (vals.count(CFG_ZONE_VALUE)) {
		    string zone_str = vals[CFG_ZONE_VALUE].as<string > ();

		    IPsubnet sn = zone.addAddressRangeList(zone_str);

		    if (sn.errored()) {
			std::cout << "Error parsing zone string: " << zone_str << std::endl;
		    }
		}

                if(vals.count(CFG_PHASE2)) {
                    string phase2str = vals[CFG_PHASE2].as<string>();

                    try {
                        isPhase2 = boost::lexical_cast<bool>(phase2str);
                    } catch( const boost::bad_lexical_cast & ) {
                        std::cout << "Error parsing boolean: " << phase2str << std::endl;
                    }
                }

                if(vals.count(CFG_EPH_PORT)) {
                    string port_str = vals[OPT_EPH_PORT].as<string>();
                    string::size_type dash = port_str.find("-");
                    string port1_str = port_str.substr(0, dash);
                    string port2_str = port_str.substr(dash + 1);

                    try {
                        _ephPortStart = boost::lexical_cast< int >(port1_str);
                        _ephPortEnd = boost::lexical_cast< int >(port2_str);
                    } catch( const boost::bad_lexical_cast & ) {
                        std::cout << "Error parsing port numbers: " << port_str << std::endl;
                    }
                }
	    }
	}
    }
}
