/* 
 * File:   NetflowViewerArguments.h
 * Author: Mike
 *
 * Created on January 18, 2010, 2:05 PM
 */

#ifndef _NETFLOWVIEWERARGUMENTS_H
#define	_NETFLOWVIEWERARGUMENTS_H

#include <string>

#include "../../shared/TimeStamp.h"
#include "../../shared/SynappArguments.h"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;
            using namespace std;

            class NetflowViewerArguments : public SynappArguments {
            public:
                /// \brief Constructor
                /// \param argc The number of arguments, including the executable name
                /// \param argv The arguments, including the executable name
                ///
                /// Construct a NetflowViewerArguments.
                explicit NetflowViewerArguments(int argc, char **argv);

                /// \brief Get the input netflows
                /// \returns the value of the input-netflow command-line argument
                const boost::filesystem::path& inputFile() const;

                const string& getSensorName() const;

                const TimeStamp& getStartTime() const;

                const TimeStamp& getEndTime() const;

                /// \brief Get the help argument
                /// \returns true if the help command-line argument was specified.
                bool isHelp() const;

                /// \brief Symptom class expects this method. So we always return true.
                bool isStdOut() const;

            private:
                /// \brief Set the options' descriptions
                ///
                /// This function populates the boost::program_options::options_description
                void setOptionsDescriptions();

                /// \brief Parse the command line options
                ///
                /// This function parses the arguments on the command line and stores their
                /// values accordingly. Sets _error is an error occurs.
                void parseOptions();

                bool parseTime(TimeStamp &t, const std::string &str);

                /// The value of the input-dir command-line option
                boost::filesystem::path _inputFile;

                /// Whether or not the help command-line option was specified
                bool _help;

                /// \brief Name of the sensor associated with the specific
                ///        execution.
                string sensor;

                /// The value of the start-time command-line option
                TimeStamp _startTime;

                /// The value of the end-time command-line option
                TimeStamp _endTime;
            };

        }
    }
}

#endif	/* _NETFLOWVIEWERARGUMENTS_H */

