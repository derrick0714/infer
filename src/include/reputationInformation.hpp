#ifndef REPUTATIONINFORMATION_H
#define REPUTATIONINFORMATION_H

#include <libpq-fe.h>
#include <string>

#include "ipInformation.h"
//#include "dnsRecords.hpp"
#include "nameResolution.h"
#include "seenIPsInformation.hpp"

/**
 *  \brief The reputations interface.
 *
 *  This class provides stage 2 modules an interface with which to query for
 *  various types of reputation information. The types of reputation
 *  information that are available are: IP, bitwise neighbors, domain, AS, and
 *  country. 
 *
 *  By default, any of this class' member functions that take an IP address as
 *  a parameter will not return an unknown reputation. Instead, if the
 *  requested reputation is unknown, the next function in the following list
 *  called with the same IP address as its parameter: ip(), neighbors(),
 *  domain(), asn(), country(). This behavior can be disabled by setting the
 *  optional "strict" parameter of any of these functions to true.
 */
class ReputationInformation {
  public:
    /**
     *	\brief Constructor
     *
     *	\param repsDB a pointer to an active postgreSQL connection to the DB
     *	 containing the reputation information.
     *	\param ipInformation a pointer to an instance of IPInformation, used
     *	 for looking up IP -> ASN and IP -> country mappings.
     *	\param nameResolution a pointer to an instance of NameResolution, used for
     *	 looking up IP <-> domain mappings.
     *	\param seenIPsInformation a pointer to an instance of
     *	 SeenIPsInformation.
     *	\param date the date in "YYYY-MM-DD" format
     */
    ReputationInformation(PGconn *repsDB, IPInformation *ipInformation, NameResolution *nameResolution, SeenIPsInformation *seenIPsInformation, std::string date);
    
    /**
     *	\brief Find the reputation for an IP address.
     *
     *	\param _ip the IP address whose reputation to look up.
     *	\param strict whether or not to return ONLY _ip's reputation, even if
     *	 it is unknown.
     *	\return The reputation of _ip, or neighbors(_ip, 1) if it is unknown.
     *	 If strict is true, the reputation of _ip is always returned, even if
     *	 it is unknown.
     */
    double ip(uint32_t _ip, bool strict = false) const;

    // returns the avg reputation for the nth bitwise neighbors of _ip
    // if unknown and nonly is true, 0 is returned, otherwise n is incrememnted (up to 8)
    // if still unknown, domain(_ip) is returned
    /** 
     *	\brief Find the reputation of bitwise neighbors of an IP address.
     *
     *	This function calculates the average reputation of the closest bitwise neighbors
     *	 of an IP address. Only neighbors that have been seen on the network
     *	 are included in this calculation.
     *	\param _ip the IP address whose neighbors to consider.
     *	\param n the distance of neighbors to start with.
     *	\param closest when done, will contatin the bit-distance of the
     *	 neighbors cosidered in the reputation calculation
     *	\param strict whether or not to return the average reputation of ONLY
     *	 _ip's bitwise neighbors, even if they are all unknown.
     *	\param todayOnly if true, only return the reputation calculated from
     *	 today's data. ie. Don't query the Neighbors table in postgreSQL if
     *	 the reputation is unknown.
     *	\param nbrs optionaly, a pointer to a vector <uint32_t> that will end up
     *	 containing the ip addresses used to calculate the neighbors reputation.
     *	\return The average reputation of _ip's n-bit neighbors that have been
     *	 seen on the network. If unknown and n < 8,
     *	 neighbors(n+1) is returned. If unknown and n >= 8,
     *	 domain(_ip) is returned. If strict is true, the average reputation of
     *	 the bitwise neighbors of _ip that have been seen on the network is
     *	 returned, even if it is unknown.
     */
    double neighbors(uint32_t _ip, uint8_t n, uint8_t *closest, bool strict = false, bool todayOnly = false, std::vector <uint32_t> *nbrs = NULL) const;

    // returns the geometric mean of the reputations of each domain this ip belongs to
    // if unknown, asn(_ip) is returned.
    /**
     *	\brief Find the geometric mean of the reputations of all domains an IP
     *	 belongs to.
     *
     *	\param _ip the IP address whose domains to consider.
     *	\param strict whether or not to return ONLY this reputation, even if it
     *	 is unknown.
     *	\return the geometric mean of the reputations of all domains this IP
     *	 is associated with. Only domains that have been seen associated with
     *	 IP address are considered. If the reputation is unknown, or if the IP
     *	 has not been seen associated with  any domains, asn(_ip) is returned.
     *	 If strict is true, the reputation is returned even if it is unknown.
     */
    double domain(uint32_t _ip, bool strict = false) const;

    // returns the reputation for domain _d, or 0 if unknown
    /**
     *	\brief Find the reputation of a domain name.
     *
     *	\param _d the domain name whose reputation to look up.
     *	\return the average reputation of all IPs associated with this domain.
     *	 Only IPs that have been seen associated with this domain are
     *	 considered.
     */
    double domain(std::string _d) const;
    
    // returns the reputation for _ip's as, or country(_ip) if it is unknown
    /**
     *	\brief Find the reputation for the AS an IP address belongs to.
     *
     *	\param _ip an IP address in the AS whose reputation to look up.
     *	\param strict whether or not to return ONLY the reputation for the AS
     *	 this IP address belongs to, even if it is unknown.
     *	\return the reputation for the AS that _ip belongs to. If it is
     *	 unknown, country(_ip) is returned. If strict is true, the AS
     *	 reputation is returned, even if it is false.
     */
    double asn(uint32_t _ip, bool strict = false) const;

    // returns the reputation for _asn, or 0 if unknown
    /**
     *	\brief Find the reputation for an AS.
     *
     *	\param _asn the number of the AS whose reputation to look up.
     *	\return the reputation for the AS represented by _asn.
     */
    double asn(uint16_t _asn) const;

    // returns the reputation for _ip's country, or 0 if it is unknown
    /**
     *	\brief Find the reputation for the country an IP address belongs to.
     *
     *	\param _ip an IP address in the country whose reputation to look up.
     *	\param strict whether or not to return ONLY the reputation for the
     *	 country this IP address is in. (this parameter is included solely for
     *	 API consistancy.
     *	\return the reputation for the country that _ip belongs to.
     */
    double country(uint32_t _ip, bool strict = false) const;
  
    // returns the reputation for _c, or 0 if unknown
    /**
     *	\brief Find the reputation of a country.
     *
     *	\param _c the ISO 3166-1 numeric country code of the country whose
     *	 reputation to look up.
     *	\return the reputation of the country represented by _c.
     */
    double country(uint16_t _c) const;

    void setIPRepsMap(std::tr1::unordered_map <uint32_t, double> *ipReps);
    
    std::tr1::unordered_map <uint32_t, double> * getIPRepsMap();

  private:
    /// \brief The postgerSQL database connection.
    PGconn *repsDB;

    /// \brief IP -> ASN and IP -> Country mappings.
    IPInformation *ipInformation;

    /// \brief IP <-> domain mappings.
    NameResolution *nameResolution;

    /// \brief Which IP addresses have been seen on the network.
    SeenIPsInformation *seenIPsInformation;

    /// \bried Today's ip reputations
    std::tr1::unordered_map <uint32_t, double> *ipReps;

    /// \brief The day for which to query reputations
    std::string date;
};

#endif
