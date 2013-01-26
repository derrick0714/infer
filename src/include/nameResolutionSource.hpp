#ifndef _NAME_RESOLUTION_SOURCE_HPP_
#define _NAME_RESOLUTION_SOURCE_HPP_

#include <string>
#include <map>
#include <tr1/unordered_set>

#include <boost/filesystem.hpp>

#include "nameResolutionSourceType.hpp"
#include "timeStamp.h"

namespace ims {
  namespace name {
    /// A typedef for the data structure to store IP addresses
    typedef std::tr1::unordered_set <uint32_t> IPSet;

    /// A typedef for the data structure to store names
    typedef std::tr1::unordered_set <std::string> NameSet;

    /// A typedef for the data structure to store name -> ip mappings
    typedef std::multimap <std::string, uint32_t> NameIPMap;

    /// \class NameResolutionSource
    /// \brief Abstract base class for sources of name resolution information
    ///
    /// All sources of name resolution information inherit this class
    class NameResolutionSource {
      public:
	/// Destructor
	/// virtual destructor
	virtual ~NameResolutionSource();

	/// \param ips The data structure for IP addresses to be added to
	/// \param name The name being looked up
	/// \brief Look up IP addresses associated with a name
	///
	/// This function adds all IP addresses associated with name to ips.
	virtual void getIPsFromName(IPSet &ips, const std::string &name) const = 0;

	/// \param names The data structure for names to be added to
	/// \param ip The IP address being looked up
	/// \brief Look up the names that an IP address is associated with
	///
	/// This function adds all names that ip has been associated with to names.
	virtual void getNamesFromIP(NameSet &names, const uint32_t &ip) const = 0;

	/// \param name The name to query for
	/// \brief Determine whether a name has any IP addresses associated with it
	/// \return true if there are any IP addresses associated with name, false otherwise
	virtual bool hasMapping(const std::string &name) const = 0;

	/// \param ip The IP address to query for
	/// \brief Determine whether an IP address is associated with any names
	/// \return true if ip is associated with any names, false otherwise
	virtual bool hasMapping(const uint32_t &ip) const = 0;

	/// \param mappings The data structure for mappings to be added to
	/// \brief Obtain all name -> ip mappings known by this source
	///
	/// This function adds all known name -> ip mappings to mappings
	virtual void getMappings(NameIPMap &mappings) const = 0;

	/// \param resolvedIPs The data structure for IP addresses to be added to
	/// \brief Obtain all IP addresses resolved in this source
	///
	/// This function adds all resolved IP addresses to resolvedIPs
	virtual void getResolvedIPs(IPSet &resolvedIPs) const = 0;

	/// \param host The host we're checking for
	/// \param resolvedIP The IP we're interested in
	/// \param t The time we're checking for
	/// \brief Check if a host resolved an ip at a certain time
	/// \return true if host resolved resolvedIP at time t
	///
	/// This function checks to see whether host host resolved IP resolvedIP at time t
	virtual bool hasHostResolvedIP(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const = 0;

	/// \param host The host we're checking for
	/// \param resolvedIP The IP we're interested in
	/// \param t The time we're checking for
	/// \brief Find the most recent time a host resolved an IP address
	/// \return the latest time host resolved resolvedIP that is <= t
	///
	/// This function returns the latest time host resolved resolvedIP that is <= t. If t is < all known times, TimeStamp(0,0) is returned.
	virtual TimeStamp getLastResolutionTime(const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t) const = 0;

	/// \param bdbFilename The name of the BDB file to use for updating mappings
	/// \param endTime The end time of the BDB file being used to update the mappings
	/// \brief Update name resolution mappings from a BDB file
	/// \return true if mappings are successfully updated, false otherwise
	virtual bool updateMappings(const boost::filesystem::path &bdbFilename) = 0;
	
	/// \brief Clear all data stored by a source
	///
	/// This function clears all data and/or mappings stored by this source
	virtual void clear() = 0;

	/// \brief Determine the source type
	/// \return the type of this source
	virtual NameResolutionSourceType getType() const = 0;

      protected:
    };

    inline NameResolutionSource::~NameResolutionSource() {
    }
  }
}

#endif
