#include <iostream>
#include <netinet/in.h>

#include "postgreSQL.h"

#include "seenIPsInformation.hpp"

#define SEEN_IPS_TABLE_SCHEMA "\"ip\" uint32 NOT NULL, \
			       PRIMARY KEY (\"ip\")"

SeenIPsInformation::SeenIPsInformation()
{}

bool SeenIPsInformation::initFromPostgreSQL(PGconn *postgreSQL, const std::string date) {
    PGresult *result;

    std::string query = "SELECT ip from \"SeenIPs\".\"";
    query += date + "\"";

    result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
	//std::cerr << "SeenIPsInformation: Failed to query SeenIPs table for date " << date << std::endl;
	PQclear(result);
	return false;
    }

    for (int i = 0; i < PQntuples(result); ++i) {
	if (!add(ntohl(*(uint32_t *)PQgetvalue(result, i, 0)))) {
	    std::cerr << "SeenIPsInformation: duplicate IP..." << std::endl;
	}
    }

    PQclear(result);
    return true;
}   

bool SeenIPsInformation::seen(uint32_t ip) const {
    return !(seenIPsList.find(ip) == seenIPsList.end());
}

bool SeenIPsInformation::add(uint32_t ip) {
    if (!seen(ip)) {
	seenIPsList.insert(ip);
	return true;
    } else {
	return false;
    }
}

uint32_t SeenIPsInformation::write(PGconn *postgreSQL, std::string date) {
    std::tr1::unordered_set <uint32_t>::iterator it;
    uint32_t ret = 0;
    size_t flushSize = 32;

    if (!preparePGTable(postgreSQL, "SeenIPs", date, SEEN_IPS_TABLE_SCHEMA)) {
	std::cerr << PQerrorMessage(postgreSQL);
	return 0;
    }

    PGBulkInserter IPInserter(postgreSQL, "SeenIPs", date.c_str(), flushSize, "%ud");

    for (it = seenIPsList.begin(); it != seenIPsList.end(); ++it) {
	if (!IPInserter.insert(NULL, *it)) {
	    std::cerr << PQerrorMessage(postgreSQL);
	    return 0;
	}
	++ret;
    }
    IPInserter.flush();

    return ret;
}   

std::tr1::unordered_set <uint32_t>::iterator SeenIPsInformation::begin() {
    return seenIPsList.begin();
}

std::tr1::unordered_set <uint32_t>::iterator SeenIPsInformation::end() {
    return seenIPsList.end();
}
