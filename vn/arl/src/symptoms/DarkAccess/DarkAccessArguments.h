/* 
 * File:   DarkAccessArguments.h
 * Author: Mikhail Sosonkin
 *
 * Created on December 2, 2009, 10:27 PM
 */

#ifndef _DARKACCESSARGUMENTS_H
#define	_DARKACCESSARGUMENTS_H

#include <string>

#include "../../shared/SynappArguments.h"
#include "../IPZone.h"
#include "DarkAccessConfiguration.h"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;
            using namespace std;

            class DarkAccessArguments : public SynappArguments {
            public:
                /// \brief Constructor
                /// \param argc The number of arguments, including the executable name
                /// \param argv The arguments, including the executable name
                ///
                /// Construct a DarkAccessArguments.
                explicit DarkAccessArguments(int argc, char **argv);

                /// \brief Get the input netflows
                /// \returns the value of the input-netflow command-line argument
                const boost::filesystem::path& inputNetFlowFile() const;

                /// \brief Get the config file
                /// \returns the value of the config-file command-line argument, if set,
                /// or an empty path otherwise.
                const boost::filesystem::path& configFile() const;

                /// \brief Get the output file
                /// \returns the value of the output-file command-line argument, if set,
                /// or temp file otherwise.
                const boost::filesystem::path& outputFile() const;

                const boost::filesystem::path& getStateFile() const;

                /// \brief Get the time window in hours.
                /// \returns the value of the time time window
                const int getTimeWindow() const;

                /// \returns the string that represents monitored subnet.
                const IPZone getMonitoredZone() const;

                const string getSensorName() const;

                /// \brief Get the help argument
                /// \returns true if the help command-line argument was specified.
                bool isHelp() const;

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

                /// The value of the input-dir command-line option
                boost::filesystem::path _inputDir;

                /// The value of the config-file command-line option
                boost::filesystem::path _configFile;

                /// The value of the output file. BDB44
                boost::filesystem::path _output;

                /// \brief filename for storing the map file.
                boost::filesystem::path _stateFile;

                /// The value of the time window
                int hours;

                /// \brief Network mask; monitored network zone
                ///  ex: 192.168.1.2/24 or 2001:480:60:55::3/96
                IPZone zone;

                /// Whether or not the help command-line option was specified
                bool _help;

                /// \brief Whether or not the user wants the output on standard out as well.
                ///  default: true. output is in human readable format.
                bool _stdout;

                /// \brief Name of the sensor associated with the specific
                ///        execution.
                string sensor;
            };

        }
    }
}

#endif	/* _DARKACCESSARGUMENTS_H */

