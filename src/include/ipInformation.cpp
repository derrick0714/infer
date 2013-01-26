#include <sys/endian.h>

#include "ipInformation.h"

bool IPInformation::initialize(PGconn* postgreSQL) {
	// load as ip intervals
	PGresult *result = PQexecParams(postgreSQL, "SELECT * FROM \"Maps\"" \
												".\"asIPBlocks\" ORDER BY \"firstIP\", " \
												"\"lastIP\" DESC", 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		return false;
	}
	uint16_t asn;
	interval<uint32_t> ival;
	for (int row = 0; row < PQntuples(result); ++row) {
		if (ival.set(be32toh(*(uint32_t*)PQgetvalue(result, row, 1)),
					 be32toh(*(uint32_t*)PQgetvalue(result, row, 2))))
		{
			asn = be16toh(*(uint16_t*)PQgetvalue(result, row, 0));
			_as_map.insert(as_map_t::value_type(ival, asn));
		}
	}
	PQclear(result);

	// load as names
	result = PQexecParams(postgreSQL, "SELECT * FROM \"Maps\"." \
									  "\"asNames\"", 0, NULL, NULL, NULL, NULL,
									  1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		return false;
	}
	for (int row = 0; row < PQntuples(result); ++row) {
		if (strlen((char*)PQgetvalue(result, row, 1)) > 0) {
			asNames.insert(std::make_pair(be16toh(*(uint16_t*)PQgetvalue(result, row, 0)),
										  std::make_pair((char*)PQgetvalue(result, row, 1),
														 (char*)PQgetvalue(result, row, 2))));
		}
		else {
			asNames.insert(std::make_pair(be16toh(*(uint16_t*)PQgetvalue(result, row, 0)),
										  std::make_pair("", (char*)PQgetvalue(result, row, 2))));
		}
	}
	PQclear(result);

	// load country ip intervals
	result = PQexecParams(postgreSQL, "SELECT * FROM \"Maps\"." \
									  "\"countryIPBlocks\"", 0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		return false;
	}
	int16_t cn;
	for (int row = 0; row < PQntuples(result); ++row) {
		if (ival.set(be32toh(*(uint32_t*)PQgetvalue(result, row, 1)),
					 be32toh(*(uint32_t*)PQgetvalue(result, row, 2))))
		{
			cn = be16toh(*(uint16_t*)PQgetvalue(result, row, 0));
			_country_map.insert(country_map_t::value_type(ival, cn));
		}
	}
	PQclear(result);

	// load country names
	result = PQexecParams(postgreSQL, "SELECT * FROM \"Maps\"." \
									  "\"countryNames\"", 0, NULL, NULL, NULL,
									  NULL, 1);
	for (int row = 0; row < PQntuples(result); ++row) {
		countryNames.insert(std::make_pair(be16toh(*(uint16_t*)PQgetvalue(result, row, 0)),
										   std::make_pair((char*)PQgetvalue(result, row, 1),
														  (char*)PQgetvalue(result, row, 2))));
	}
	PQclear(result);
	return true;
}

uint16_t IPInformation::getASN(uint32_t ip) {
	as_map_t::const_iterator it(_as_map.find(ip));
	if (it == _as_map.end()) {
		return 0;
	}

	return it->second;
}

const std::string &IPInformation::getASName(uint16_t asn) {
	return asNames[asn].first;
}

const std::string &IPInformation::getASDescription(uint16_t asn) {
	return asNames[asn].second;
}

int16_t IPInformation::getCountry(uint32_t ip) {
	country_map_t::const_iterator it(_country_map.find(ip));
	if (it == _country_map.end()) {
		return 0;
	}

	return it->second;
}

const std::string &IPInformation::getCountryCode(int16_t countryNumber) {
	return countryNames[countryNumber].first;
}

const std::string &IPInformation::getCountryName(int16_t countryNumber) {
	return countryNames[countryNumber].second;
}
