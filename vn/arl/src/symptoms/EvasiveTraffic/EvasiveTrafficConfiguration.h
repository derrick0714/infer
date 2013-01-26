/* 
 * File:   EvasiveTrafficConfiguration.h
 * Author: Mike
 *
 * Created on December 10, 2009, 10:32 PM
 */

#ifndef _EVASIVETRAFFICCONFIGURATION_H
#define	_EVASIVETRAFFICCONFIGURATION_H

#include <boost/filesystem.hpp>

#include "../../shared/SampleSynappConfiguration.h"

#define DEFAULT_TTL_VALUE 10

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;

            class EvasiveTrafficConfiguration : public SynappConfiguration {
            public:
                EvasiveTrafficConfiguration();

                EvasiveTrafficConfiguration(const boost::filesystem::path &fileName);

                const int getTTL() const;
            private:
                virtual void setOptionsDescriptions();

                virtual void parseOptions();

                int ttl;
            };

        }
    }
}

#endif	/* _EVASIVETRAFFICCONFIGURATION_H */

