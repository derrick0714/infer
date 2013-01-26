/* 
 * File:   FrequentRebootsArguments.h
 * Author: Mikhail Sosonkin
 *
 * Created on December 2, 2009, 10:27 PM
 */

#ifndef _DARKACCESSARGUMENTS_H
#define	_DARKACCESSARGUMENTS_H

#include <string>

#include "../../shared/SynappArguments.h"
#include "../IPZone.h"
#include "FrequentRebootsConfiguration.h"
#include "HIEAppStore.hpp"

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace vn::arl::shared;
            using namespace std;

            class FrequentRebootsArguments : public SynappArguments {
            public:
                /// \brief Constructor
                /// \param argc The number of arguments, including the executable name
                /// \param argv The arguments, including the executable name
                ///
                /// Construct a FrequentRebootsArguments.
                explicit FrequentRebootsArguments(int argc, char **argv);

                ~FrequentRebootsArguments();

                /// \brief Get the input netflows
                /// \returns the value of the input-netflow command-line argument
                const boost::filesystem::path& inputNetFlowFile() const;

                const boost::filesystem::path& inputPcapFile() const;

                /// \brief Get the config file
                /// \returns the value of the config-file command-line argument, if set,
                /// or an empty path otherwise.
                const boost::filesystem::path& configFile() const;

                /// \brief Get the output file
                /// \returns the value of the output-file command-line argument, if set,
                /// or temp file otherwise.
                const boost::filesystem::path& outputFile() const;

                HIEAppStore& getApplicationStore() const;

                const boost::filesystem::path& getHostAppMapFile() const;

                const boost::filesystem::path& getHostRebootFile() const;

                const string getSensorName() const;

                /// \brief Get the help argument
                /// \returns true if the help command-line argument was specified.
                bool isHelp() const;

                bool isStdOut() const;

                bool hasPcap() const;

                int getEventsAtReboot() const;

                /// \brief Seconds of the window for the events to be considered
                ///        in the reboot.
                int getBootTimeWindow() const;

                int getRebootCount() const;

                /// \brief Seconds of the window to count the reboots are too
                ///        frequent.
                int getWatchTime() const;

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

                /// Object that stores the Host Initiation Events definitions.
                HIEAppStore* appStore;

                /// The value of the input-dir command-line option
                boost::filesystem::path _inputDir;

                boost::filesystem::path _inputPcap;

                /// The value of the config-file command-line option
                boost::filesystem::path _configFile;

                /// The value of the output file. BDB44
                boost::filesystem::path _output;

                /// \brief filename for storing the map file. host -> HIE set
                boost::filesystem::path _hostAppMap;

                /// \brief filename for storing the map file. host -> reboot times set
                boost::filesystem::path _hostRebootMap;

                /// \brief How many events constitute a reboot
                int eventsAtReboot;

                /// \brief At what time window should the events happen to declare
                ///        a reboot
                int bootTimeWindow;

                /// \brief How many reboots should there be before we flag the
                ///        host
                int rebootCount;

                /// \brief At what time window should the rebootCount events
                ///        happen
                int watchTime;

                /// Whether or not the help command-line option was specified
                bool _help;

                /// \brief Whether or not the user wants the output on standard out as well.
                ///  default: true. output is in human readable format.
                bool _stdout;

                bool _hasPcap;

                /// \brief Name of the sensor associated with the specific
                ///        execution.
                string sensor;
            };

        }
    }
}

#endif	/* _DARKACCESSARGUMENTS_H */

