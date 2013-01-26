#ifndef DNSRECORDS_HPP
#define DNSRECORDS_HPP

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <string>
#include <vector>

template <typename key, typename value>
struct bidirectional_unordered_multimap {
    typedef std::pair <key, value> value_type;
    typedef boost::multi_index::multi_index_container
            <
		value_type,
		boost::multi_index::indexed_by
		<
		    boost::multi_index::hashed_non_unique
		    <
			boost::multi_index::member <value_type, key, &value_type::first>
		    >,
		    boost::multi_index::hashed_non_unique
		    <   
                        boost::multi_index::member <value_type, value, &value_type::second>
                    >
                >
            > type;
};

/**
 *  \brief The dns mappings interface.
 *
 *  This class provides the interface for IP <-> domain lookups.
 */
class DNSRecords {
  public:
    /**
     *	\brief Constructor.
     */
    DNSRecords(bidirectional_unordered_multimap <uint32_t, std::string>::type *records);

    /**
     *	\brief Find IP addresses associated with a domain.
     *
     *	\param name the domain name to look up.
     *	\param ips a vector in which to store the requested IP addresses.
     */
    void nameLookup(std::string name, std::vector <uint32_t> &ips) const;

    /**
     *	\brief Find domains an IP address is associated with.
     *
     *	\param ip the IP address whose domains to look up.
     *	\param names a vector in which to store the requested domains.
     */
    void ipLookup(uint32_t ip, std::vector <std::string> &names) const;

    /**
     *	\brief Get a pointer to the bitirectional_unordered_multimap containing
     *	 the IP <-> domain mappings.
     *
     *	DO NOT use this unless you are absolutely sure you know what you're doing.
     */
    bidirectional_unordered_multimap <uint32_t, std::string>::type * getMultimap();

  private:
    /**
     *	\brief The IP <-> domain mappings.
     */
    bidirectional_unordered_multimap <uint32_t, std::string>::type *records;
};

#endif
