/*
 * Provides autonomous system and country information for IPv4 addresses. The
 * initialize(PGconn*) function pulls down the appropriate data from PostgreSQL,
 * returning true upon success or false upon failure. The
 * getASN(const uint32_t&) function returns the ASN of its parameter, or 0 if
 * none is found. The getCountry(const uint32_t&) function returns the
 * ISO 3166-1 numeric country code of its parameter, or 0 if none is found.
 * Initial queries run in logarithmic time, but subsequent queries for the same
 * IP run in constant time, as results are cached in a hash table.
 */

#ifndef INFER_INCLUDE_IPINFORMATION_H_
#define INFER_INCLUDE_IPINFORMATION_H_

#include "postgreSQL.h"
#include "interval.hpp"
#include "interval_map.hpp"

class IPInformation {
  public:
	uint16_t getASN(uint32_t ip);
	const std::string & getASName(uint16_t asn);
	const std::string & getASDescription(uint16_t asn);
	int16_t getCountry(uint32_t ip);
	const std::string &getCountryCode(int16_t countryNumber);
	const std::string &getCountryName(int16_t countryNumber);
	bool initialize(PGconn*);
  private:
	typedef interval_map<interval<uint32_t>, uint16_t> as_map_t;
	typedef interval_map<interval<uint32_t>, int16_t> country_map_t;

	bool error;
	as_map_t _as_map;
	std::tr1::unordered_map<uint16_t, std::pair<std::string, std::string> >
		asNames;
	country_map_t _country_map;
	std::tr1::unordered_map<int16_t, std::pair<std::string, std::string> >
		countryNames;
};

#endif
