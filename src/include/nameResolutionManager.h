#ifndef _NAME_RESOLUTION_MANAGER_H_
#define _NAME_RESOLUTION_MANAGER_H_

#include <string>
#include <vector>
#include <tr1/unordered_set>
#include <map>

#include <boost/filesystem.hpp>

#include "nameResolutionSource.hpp"

namespace ims {
namespace name {

/// \class NameResolutionManager
/// \brief The main name resolution query interface.
///
/// This class is the primary interface for querying name resolution
/// information. It manages an arbitrary number of sources of an arbitrary
/// number of source types, and allows for querying only sources of a 
/// specific type, or all sources.
class NameResolutionManager {
  public:
/// \param ips The data structure for IP addresses to be added to
/// \param name The name being looked up
/// \param sourceType The type of sources to query
/// \brief Look up IP addresses associated with a name
///
/// This function adds to ips all IP addresses associated with name that
/// are in a source of type sourceType.
void getIPsFromName(IPSet &ips, const std::string &name, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param names The data structure for names to be added to
/// \param ip The IP address being looked up
/// \param sourceType The type of sources to query
/// \brief Look up the names that an IP address is associated with
///
/// This function adds to names all names that ip has been associated 
/// with in a source of type sourceType.
void getNamesFromIP(NameSet &names, const uint32_t &ip, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param name The name to query for
/// \param sourceType The type of sources to query
/// \brief Determine whether a name has any IP addresses associated with it in a source of type sourceType
/// \return true if there are any IP addresses associated with name in a source of type sourceType, false otherwise
bool hasMapping(const std::string &name, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param ip The IP address to query for
/// \param sourceType The type of sources to query
/// \brief Determine whether an IP address is associated with any names in a source of type sourceType
/// \return true if ip is associated with any names in a source of type sourceType, false otherwise
bool hasMapping(const uint32_t &ip, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param mappings The data structure for mappings to be added to
/// \param sourceType The type of sources to query
/// \brief Obtain all name -> ip mappings known by this source
///
/// This function adds all known name -> ip mappings of type sourceType
/// to mappings
void getMappings(NameIPMap &mappings, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param resolvedIPs The data structure for IP addresses to be added to
/// \param sourceType The type of sources to query
/// \brief Obtain all IP addresses resolved in this source
///
/// This function adds all resolved IP addresses of type sourceType
/// to resolvedIPs
void getResolvedIPs(IPSet &resolvedIPs, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param host The host we're checking for
/// \param resolvedIP The IP we're interested in
/// \param t The time we're checking for
/// \param sourceType The type of sources to query
/// \brief Check if a host resolved an ip at a certain time
/// \return true if host resolved resolvedIP at time t
///
/// This function checks to see whether host host resolved IP resolvedIP at time t using a source of type sourceType
bool hasHostResolvedIP(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param host The host we're checking for
/// \param resolvedIP The IP we're interested in
/// \param t The time we're checking for
/// \brief Find the most recent time a host resolved an IP address
/// \return the latest time host resolved resolvedIP that is <= t
///
/// This function returns the latest time host resolved resolvedIP that is <= t. If t is < all known times, TimeStamp(0,0) is returned.
TimeStamp getLastResolutionTime(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, NameResolutionSourceType sourceType = NameResolutionSourceType::ANY) const;

/// \param source A pointer to the NameResolutionSource that is to be managed
/// \brief Specify a NameResolutionSource to be managed
///
/// This function adds a new NameResolutionSource to the Manager. This
/// new source can now be queried, when appropriate, when the Manager 
/// is queried.
void addSource(NameResolutionSource *source);

/// \param bdbFilename The name of the BDB file to use for updating mappings
/// \param endTime The end time of the BDB file being used to update the mappings
/// \param sourceType The type of sources to be updated using this file
/// \brief Update name resolution mappings of a certain type from a BDB file
/// \note NameResolutionSourceType::ANY is not a valid value for sourceType
/// \return true if all sources of type sourceType are successfully
/// updated, false otherwise
bool updateMappings(const boost::filesystem::path &file, NameResolutionSourceType sourceType);

/// \param sourceType The type of sources to clear
/// \brief Clear all data stored by sources of type sourceType
///
/// Clears all data stored by sources of type sourceType
void clear(NameResolutionSourceType sourceType = NameResolutionSourceType::ANY);

  private:
/// A typedef for the data structure to store pointers to the
/// NameResolutionSources that can be queried
typedef std::multimap <NameResolutionSourceType, NameResolutionSource *> SourcesMap;
SourcesMap sources; /// Stores pointers to the NameResolutionSources that can be queried
};

}
}

#endif
