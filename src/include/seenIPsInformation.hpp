#ifndef SEENIPSINFORMATION_HPP
#define SEENIPSINFORMATION_HPP

#include <tr1/unordered_set>
#include <libpq-fe.h>

/**
 *  \brief The seen IPs interface.
 *
 *  This is used by analysis modules that need an answer to the question "Has
 *  address X been seen sending or receiving traffic on the network?" The stage
 *  1 "seenIPs" module is responsible for populating the set of seen IPs, so
 *  this interface should only be used by stage 2 modules.
 */
class SeenIPsInformation {
  public:
    /**
     *	\brief Default constructor.
     *
     *	Creates an empty SeenIPsInformation
     */
    SeenIPsInformation();

    /**
     *	\brief init seen IPs from postgreSQL.
     *
     *	Populates SeenIPsInformation from postrgeSQL with seen IPs from date.
     *	\param postgreSQL an open postgreSQL connection
     *	\param date the day whose seen IPs should be loaded
     */
    bool initFromPostgreSQL(PGconn *postgreSQL, const std::string date);

    /**
     *	\brief Finds out whether an IP has been seen on the network.
     *
     *	\param ip the IP address to be queried for
     *	\return true if IP has been seen on the network, false otherwise.
     */
    bool seen(uint32_t ip) const;

    /**
     *	\brief Adds an IP address to the set of seen IPs.
     *
     *	This function should only be called for IP addresses that have actually
     *	 been seen on the network.
     *	\param ip the IP address to be added
     *	\return true if added, false is already seen
     */
    bool add(uint32_t ip);

    /**
     *	\brief Writes seen IPs list to postgreSQL
     *
     *	Writes the seenIPs list to the table "SeenIPs".date
     *	\param postgreSQL an open postgreSQL connection
     *	\param date the day for which to record these seenIPs
     *	\return the number of IPs inserted into the DB
     */
    uint32_t write(PGconn *postgreSQL, std::string date);

    /**
     *	\brief Returns an iterator to the beginning of the seenIPsList
     *
     *	\return seenIPsList.begin()
     */
    std::tr1::unordered_set <uint32_t>::iterator begin();

    /**
     *	\brief Returns an iterator to the end of the seenIPsList
     *
     *	\return seenIPsList.end()
     */
    std::tr1::unordered_set <uint32_t>::iterator end();

  private:
    /**
     *	\brief The set of IP addresses that have been seen on the network.
     *
     *	The "seenIPs" module populates this in stage 1.
     */
    std::tr1::unordered_set <uint32_t> seenIPsList;
};

#endif
