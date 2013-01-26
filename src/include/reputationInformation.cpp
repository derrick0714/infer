#include <iostream>
#include <sys/endian.h>
#include <sstream>
#include <vector>

#include "reputationInformation.hpp"

ReputationInformation::ReputationInformation(PGconn *repsDB,
					     IPInformation *ipInformation,
					     NameResolution *nameResolution,
					     SeenIPsInformation *_seenIPsInformation,
					     std::string date) 
    : repsDB(repsDB), ipInformation(ipInformation), nameResolution(nameResolution), date(date)
{
    seenIPsInformation = _seenIPsInformation;
    ipReps = NULL;
}


double ReputationInformation::ip(uint32_t _ip, bool strict) const {
    if (ipReps != NULL) {
	// use the in memory map to get ip reputations
	std::tr1::unordered_map <uint32_t, double>::iterator it;
	if ((it = ipReps -> find(_ip)) == ipReps -> end()) {
	    if (strict) {
		return 0;
	    } else {
		uint8_t tmp;
		return neighbors(_ip, 1, &tmp);
	    }
	}
	return it -> second;
    }
    // else get it from postgres

    PGresult *result;
    uint64_t tmp;
    double rep;

    std::ostringstream query;
    query << "SELECT \"reputation\" FROM \"IPReputations\".\"" << date
	  << "\" WHERE \"ip\" = '" << _ip << "'";

    result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	if (PQntuples(result) > 0) {
	    // reputation found...
	    tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
	    memmove(&rep, &tmp, 8);

	    PQclear(result);

	    return rep;
	} else {
	    // unknown reputation...
	    //std::cerr << "Unknown rep for ip...getting asn rep." << std::endl;

	    PQclear(result);

	    if (strict) {
		return 0;
	    } else {
		uint8_t tmp;
		return neighbors(_ip, 1, &tmp);
	    }
	}
    } else {
	std::cerr << "ReputationInformation::ip(): Error querying DB." << std::endl;
	PQclear(result);
	return 0;
    }
}


double ReputationInformation::neighbors(uint32_t _ip, uint8_t n, uint8_t *closest, bool strict, bool todayOnly, std::vector <uint32_t> *nbrs) const {
    std::vector <uint32_t> *_nbrs;
    uint32_t mask, ip;
    double rep = 0;
    bool mine;

    if (nbrs != NULL) {
	_nbrs = nbrs;
	_nbrs -> clear();
	mine = false;
    } else {
	_nbrs = new std::vector <uint32_t>();
	mine = true;
    }

    mask = 0xffffffff;
    mask <<= n;

    for (ip = _ip & mask; (ip & mask) == (_ip & mask); ++ip) {
	if (ip != _ip && seenIPsInformation -> seen(ip)) {
	    _nbrs -> push_back(ip);
	}
    }

    if (_nbrs -> size()) {
	for (size_t i = 0; i < _nbrs -> size(); ++i) {
	    rep += this -> ip((*_nbrs)[i], true);
	}
	rep /= _nbrs -> size();
	
	if (mine) {
	    delete _nbrs;
	}
	
	*closest = n;
	return rep;

    }

    if (mine) {
	delete _nbrs;
	_nbrs = NULL;
    }

    if (n < 8) {
	return neighbors(_ip, n + 1, closest, strict, todayOnly, _nbrs);
    }

    *closest = 0;

    if (strict) {
	if (todayOnly) {
	    return 0;
	}

	// lookup 8-bit neighbor in table, else return 0
	PGresult *result;
	std::ostringstream query;
	query << "SELECT reputation FROM \"NeighborReputations\".\"" <<  date << "\" WHERE \"ip\" = '" << _ip << "'";
	uint64_t tmp;
	
	result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	    if (PQntuples(result) > 0) {
		tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
		memmove(&rep, &tmp, 8);
		PQclear(result);
		*closest = 8;
		return rep;
	    }
	}
	    
	PQclear(result);

	return 0;
    }

    return domain(_ip);
}


double ReputationInformation::domain(uint32_t _ip, bool strict) const {
    ims::name::NameSet names;
    double rep = 1;
    double tmp;
    size_t seen = 0;

    nameResolution -> getNamesFromIP(names, _ip);

    if (!names.size()) {
	if (strict) {
	    return 0;
	}

	return asn(_ip);
    }

    for (ims::name::NameSet::iterator it = names.begin();
	 it != names.end();
	 ++it)
    {
	if ((tmp = domain(*it)) != 0.) {
	    rep *= fabs(tmp);
	    ++seen;
	}
    }

    if (seen) {
	rep = pow(rep, 1.0 / seen);
	rep *= -1;
    } else {
	rep = 0;
    }

    if (rep || strict) {
	return rep;
    }

    return asn(_ip);
}


double ReputationInformation::domain(std::string _d) const {
    ims::name::IPSet ips;
    double rep = 0;

    nameResolution -> getIPsFromName(ips, _d);

    if (!ips.size()) {
	return rep;
    }
    
    //for (size_t i = 0; i < ips.size(); ++i) {
    for (ims::name::IPSet::iterator it = ips.begin();
	 it != ips.end();
	 ++it)
    {
	rep += ip(*it, true);
    }

    rep /= ips.size();

    return rep;
}


double ReputationInformation::asn(uint32_t _ip, bool strict) const {
    PGresult *result;
    uint64_t tmp;
    double rep;

    std::ostringstream query;
    query << "SELECT \"reputation\" FROM \"ASReputations\".\"" << date
	  << "\" WHERE \"asn\" = '" << ipInformation -> getASN(_ip) << "'";

    result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	if (PQntuples(result) > 0) {
	    // reputation found...
	    tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
	    memmove(&rep, &tmp, 8);

	    PQclear(result);
	    return rep;
	} else {
	    // unknown reputation...
	    //std::cerr << "Unknown rep for asn...getting country rep." << std::endl;
	    PQclear(result);

	    if (strict) {
		return 0;
	    }

	    return country(_ip);
	}
    } else {
	std::cerr << "ReputationInformation::asn(): Error querying DB." << std::endl;
	PQclear(result);
	return 0;
    }
}


double ReputationInformation::asn(uint16_t _asn) const {
    PGresult *result;
    uint64_t tmp;
    double rep;

    std::ostringstream query;
    query << "SELECT \"reputation\" FROM \"ASReputations\".\"" << date 
	  << "\" WHERE \"asn\" = '" << _asn << "'";

    result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	if (PQntuples(result) > 0) {
	    // reputation found...
	    tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
	    memmove(&rep, &tmp, 8);

	    PQclear(result);
	    return rep;
	} else {
	    // unknown reputation...
	    //std::cerr << "Unknown rep for asn...getting country rep." << std::endl;
	    PQclear(result);
	    return 0;
	}
    } else {
	std::cerr << "ReputationInformation::asn(): Error querying DB." << std::endl;
	PQclear(result);
	return 0;
    }
}


double ReputationInformation::country(uint32_t _ip, bool strict) const {
    PGresult *result;
    uint64_t tmp;
    double rep;

    std::ostringstream query;
    query << "SELECT \"reputation\" FROM \"CountryReputations\".\"" << date 
	  << "\" WHERE \"country\" = '" << ipInformation -> getCountry(_ip) << "'";

    result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	if (PQntuples(result) > 0) {
	    // reputation found...
	    tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
	    memmove(&rep, &tmp, 8);

	    PQclear(result);
	    return rep;
	} else {
	    // unknown reputation...
	    //std::cerr << "Unknown rep for country." << std::endl;
	    PQclear(result);
	    return 0;
	}
    } else {
	std::cerr << "ReputationInformation::country(): Error querying DB." << std::endl;
	PQclear(result);
	return 0;
    }
}


double ReputationInformation::country(uint16_t _c) const {
    PGresult *result;
    uint64_t tmp;
    double rep;

    std::ostringstream query;
    query << "SELECT \"reputation\" FROM \"CountryReputations\".\"" << date
	  << "\" WHERE \"country\" = '" << _c << "'";

    result = PQexecParams(repsDB, query.str().c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
	if (PQntuples(result) > 0) {
	    // reputation found...
	    tmp = be64toh(*(uint64_t *) PQgetvalue(result, 0, 0));
	    memmove(&rep, &tmp, 8);

	    PQclear(result);
	    return rep;
	} else {
	    // unknown reputation...
	    //std::cerr << "Unknown rep for country." << std::endl;
	    PQclear(result);
	    return 0;
	}
    } else {
	std::cerr << "ReputationInformation::country(): Error querying DB." << std::endl;
	PQclear(result);
	return 0;
    }
}

void ReputationInformation::setIPRepsMap(std::tr1::unordered_map <uint32_t, double> *ipReps) {
    this -> ipReps = ipReps;
}

std::tr1::unordered_map <uint32_t, double> * ReputationInformation::getIPRepsMap() {
    return ipReps;
}
