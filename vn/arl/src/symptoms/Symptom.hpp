/* 
 * File:   Symptom.hpp
 * Author: Mike
 *
 * Created on January 9, 2010, 5:30 PM
 */

#ifndef _SYMPTOM_HPP
#define	_SYMPTOM_HPP

#include <iostream>
#include <string>

#include "../shared/ProtocolNumbers.hpp"
#include "../shared/arl_parsing/NetFlowARLRecord.h"
#include "../shared/TimeStamp.h"

#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/date_time/posix_time/posix_time_io.hpp>

#define OUT_FIELD_SEP "||"

namespace vn {
    namespace arl {
        namespace symptom {

	    using namespace std;
	    using namespace vn::arl::shared;
	    using namespace boost::posix_time;

            template <typename ArgumentsType>
            class Symptom {
            public:
                explicit Symptom(const ArgumentsType& args, const string sym_name, const string sen_name);

                /// \brief execute the Symptom detection algorithm.
                virtual int run() = 0;

                /// \brief Output a flow in ARL specific format.
                virtual void arl_output(const NetFlowARLRecord& rec, const string& comment);

            protected:
                const ArgumentsType& s_args;
                const string symptom_name;
                const string sensor_name;
                bool dostdout;
            };

            template <typename ArgumentsType>
            Symptom<ArgumentsType>::Symptom(const ArgumentsType& args, const string s_name, const string sen_name):
            s_args(args), symptom_name(s_name), sensor_name(sen_name) {
                dostdout = s_args.isStdOut();
            }

            template <typename ArgumentsType>
            void Symptom<ArgumentsType>::arl_output(const NetFlowARLRecord& rec, const string& comment) {
                // example:
                //  ipad||gator-arl691||20090814||19||34||53||246214||131.218.128.14||50061||74.217.240.81||80||TCP|| ||-1||-1|| ||HTTP protocol anomaly
                if(dostdout) {
                    using namespace boost::gregorian;
                    using namespace boost::posix_time;

                    std::stringstream out;

                    ptime t = (ptime) rec.startTime();
                    date d = t.date();

                    time_facet* facet = new time_facet(string("%Y%m%d").append(OUT_FIELD_SEP).append("%H").append(OUT_FIELD_SEP)
                                                        .append("%M").append(OUT_FIELD_SEP).append("%S").append(OUT_FIELD_SEP)
                                                        .append("%f").c_str());

                    out.imbue(locale(out.getloc(), facet));

                    out << symptom_name << OUT_FIELD_SEP
                        << sensor_name << OUT_FIELD_SEP
                        << t << OUT_FIELD_SEP
                        << rec.getClientIP().to_string() << OUT_FIELD_SEP
                        << rec.getClientPort() << OUT_FIELD_SEP
                        << rec.getServerIP().to_string() << OUT_FIELD_SEP
                        << rec.getServerPort() << OUT_FIELD_SEP
                        << getProtocolString(rec.getProtocol()) << OUT_FIELD_SEP
                        << " " << OUT_FIELD_SEP << -1 << OUT_FIELD_SEP << -1 << OUT_FIELD_SEP // I haven't fugured out, yet, what these fields are
                        << comment;

                    cout << out.str() << endl;
                }
            }
        }
    }
}

#endif	/* _SYMPTOM_H */

