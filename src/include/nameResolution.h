#ifndef NAMERESOLUTION_H
#define NAMERESOLUTION_H

#include <string>

#include "nameResolutionSource.hpp"
#include "nameResolutionSourceType.hpp"
#include "queryManagerConfiguration.h"
#include "queryClient.hpp"

class NameResolution : public QueryClient {
  public:
    NameResolution(std::string sockPath, size_t sockTimeout);

    /// \param ips The data structure for IP addresses to be added to
    /// \param name The name being looked up
    /// \param sourceType The type of sources to query
    /// \brief Look up IP addresses associated with a name
    /// \return true if the query succeded, false otherwise
    ///
    /// This function adds to ips all IP addresses associated with name that
    /// are in a source of type sourceType.
    bool getIPsFromName(ims::name::IPSet &ips, const std::string &name, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);
    
    /// \param names The data structure for names to be added to
    /// \param ip The IP address being looked up
    /// \param sourceType The type of sources to query
    /// \brief Look up the names that an IP address is associated with
    /// \return true if the query succeded, false otherwise
    ///
    /// This function adds to names all names that ip has been associated 
    /// with in a source of type sourceType.
    bool getNamesFromIP(ims::name::NameSet &names, const uint32_t &ip, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param ret will be set to true if there are any IP addresses associated with name in a source of type sourceType, false otherwise
    /// \param name The name to query for
    /// \param sourceType The type of sources to query
    /// \brief Determine whether a name has any IP addresses associated with it in a source of type sourceType
    /// \return true if the query succeded, false otherwise
    bool hasMapping(bool &ret, const std::string &name, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param ret will be set to true if ip is associated with any names in a source of type sourceType, false otherwise
    /// \param ip The IP address to query for
    /// \param sourceType The type of sources to query
    /// \brief Determine whether an IP address is associated with any names in a source of type sourceType
    /// \return true if the query succeded, false otherwise
    bool hasMapping(bool &ret, const uint32_t &ip, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    bool getMappings(ims::name::NameIPMap &mappings, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param resolvedIPs The data structure for IP addresses to be added to
    /// \param sourceType The type of sources to query
    /// \brief Obtain all IP addresses resolved in this source
    /// \return true if the query succeded, false otherwise
    ///
    /// This function adds all resolved IP addresses of type sourceType
    /// to resolvedIPs
    bool getResolvedIPs(ims::name::IPSet &resolvedIPs, ims::name::NameResolutionSourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param ret will be set to true if host resolved resolvedIP at time t
    /// \param host The host we're checking for
    /// \param resolvedIP The IP we're interested in
    /// \param t The time we're checking for
    /// \param sourceType The type of sources to query
    /// \brief Check if a host resolved an ip at a certain time
    /// \return true if the query succeded, false otherwise
    ///
    /// This function checks to see whether host host resolved IP resolvedIP at time t
    bool hasHostResolvedIP(bool &ret, const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, ims::name::NameResolutionSourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param ret will be set to the latest time host resolved resolvedIP that is <= t
    /// \param host The host we're checking for
    /// \param resolvedIP The IP we're interested in
    /// \param t The time we're checking for
    /// \brief Find the most recent time a host resolved an IP address
    /// \return true if the query succeded, false otherwise
    ///
    /// This function returns the latest time host resolved resolvedIP that is <= t. If t is < all known times, TimeStamp(0,0) is returned.
    bool getLastResolutionTime(TimeStamp &ret, const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param sourceType The type of sources to clear
    /// \brief Clear all data stored by sources of type sourceType
    /// \return true if successful, false otherwise
    ///
    /// Clears all data stored by sources of type sourceType
    bool clear(ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::ANY);

    /// \param t time of data to load
    /// \brief Load specific data
    /// \param sourceType the type of data to load
    /// \return true if successfil, false otherwise
    ///
    /// Causes the dns store to load data from the update interval immesiately prior to time t
    bool load(const TimeStamp &begin, const TimeStamp &end, ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::DNS);

    /// \brief Waits for all data to be loaded by the query manager
    /// \return true if successful, false otherwise
    bool waitForData(ims::name::NameResolutionSourceType sourceType = ims::name::NameResolutionSourceType::DNS);

    /// \brief Gets the current configuration parameters from the query manager
    /// \return true if successful, false otherwise
    bool getConfiguration(ims::QueryManagerConfiguration &queryManConfig);

  private:
    char buf[0xffff];
    uint16_t cmdLen;
    std::string tmpName;
    unsigned char nameLen;
    uint32_t ip;
};

#endif
