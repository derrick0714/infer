/* 
 * File:   DNSFreeFlowsConfiguration.h
 * Author: Mike
 *
 * Created on December 10, 2009, 10:32 PM
 */

#ifndef _DNSFREEFLOWSCONFIGURATION_H
#define	_DNSFREEFLOWSCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "../../shared/SampleSynappConfiguration.h"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;

            class DNSFreeFlowsConfiguration : public SynappConfiguration {
            public:
                DNSFreeFlowsConfiguration();

                DNSFreeFlowsConfiguration(const boost::filesystem::path &fileName);

                const int getTimeOut() const;

                const boost::filesystem::path& getStateFile() const;
            private:
                virtual void setOptionsDescriptions();

                virtual void parseOptions();

                boost::filesystem::path state;

                int timeout;
            };

        }
    }
}

#endif

