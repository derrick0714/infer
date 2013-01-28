#include <iostream>
#include <string>
#include <vector>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <sys/endian.h>

#include "configuration.hpp"
#include "stringHelpers.h"
#include "address.h"
#include "timeStamp.h"
#include "sqlTime.h"
#include "hostPair.hpp"
#include "PostgreSQLConnection.hpp"
#include "postgreSQL.h"
#include "clock.hpp"
#include "bsdProcessStats.h"
#include "dns.h"
#include "roles.h"

#define PROCESS_NAME "infer_interestingIPs"
#define INTERESTING_IPS_TABLE_SCHEMA "\"ip\" uint32 NOT NULL, \
									\"names\" TEXT[] NOT NULL, \
									\"steppingStoneScore\" uint32 NOT NULL, \
									\"muleContactScore\" uint32 NOT NULL, \
									\"infectedContactScore\" uint32 NOT NULL, \
									\"evasiveTrafficScore\" uint32 NOT NULL, \
									\"darkSpaceSourceScore\" uint32 NOT NULL, \
									\"darkSpaceTargetScore\" uint32 NOT NULL, \
									\"nonDNSTrafficScore\" uint32 NOT NULL, \
									\"rebootScore\" uint32 NOT NULL, \
									\"malwareSourceScore\" uint16 NOT NULL, \
									\"malwareTargetScore\" uint16 NOT NULL, \
									\"minVirulence\" DOUBLE PRECISION NOT NULL, \
									\"maxVirulence\" DOUBLE PRECISION NOT NULL, \
									\"currentVirulence\" DOUBLE PRECISION NOT NULL, \
									\"rank\" uint32 NOT NULL, \
									PRIMARY KEY (\"ip\")"

using namespace std;
using namespace tr1;

uint16_t dangerousRoles[] = {
	SSH_BRUTE_FORCER,
	TELNET_BRUTE_FORCER,
	MICROSOFT_SQL_BRUTE_FORCER,
	ORACLE_SQL_BRUTE_FORCER,
	MYSQL_BRUTE_FORCER,
	POSTGRESQL_BRUTE_FORCER,
	DARK_SPACE_BOT,
	SPAM_BOT,
	SCAN_BOT
};

const size_t dangerousRolesCount(sizeof(dangerousRoles) / sizeof(uint16_t));

class InfectionStats {
	public:
		InfectionStats() {
			steppingStoneScore = 0;
			muleContactScore = 0;
			infectedContactScore = 0;
			evasiveTrafficScore = 0;
			darkSpaceSourceScore = 0;
			darkSpaceTargetScore = 0;
			nonDNSTrafficScore = 0;
			rebootScore = 0;
			malwareSourceScore = 0;
			malwareTargetScore = 0;
			minVirulence = 0;
			currentVirulence = 0;
		}
		uint32_t steppingStoneScore;
		uint32_t muleContactScore;
		uint32_t infectedContactScore;
		uint32_t evasiveTrafficScore;
		uint32_t darkSpaceSourceScore;
		uint32_t darkSpaceTargetScore;
		uint32_t nonDNSTrafficScore;
		uint32_t rebootScore;
		uint32_t malwareSourceScore;
		uint32_t malwareTargetScore;
		double minVirulence;
		double maxVirulence;
		double currentVirulence;
};

bool getInterestingIPDays(PGconn* postgreSQL, string tableName,
													vector <string> &interestingIPDays) {
	PGresult* result;
	string query = "SELECT \"table_name\" FROM \"information_schema\"." \
								 "\"tables\" WHERE \"table_schema\" = '" \
								 INTERESTING_IPS_SCHEMA_NAME \
								 "' AND \"table_type\" = 'BASE TABLE' AND " \
								 "\"table_name\" < '" + tableName + "' ORDER BY " \
								 "\"table_name\" DESC LIMIT 13";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		interestingIPDays.push_back((char*)PQgetvalue(result, index, 0));
	}
	return true;
}

bool getSteppingStoneScores(PGconn* postgreSQL, string tableName,
														unordered_map <uint32_t, InfectionStats> &interestingIPs,
														InfectionStats &aggregateStats) {
	PGresult* result;			 
	uint32_t steppingStoneScore;
	string query = "SELECT \"ip\", COUNT(*) FROM \"" \
								 STEPPING_STONES_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"ip\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		steppingStoneScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result,
																																	index, 1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].
			steppingStoneScore = steppingStoneScore;
		aggregateStats.steppingStoneScore += steppingStoneScore;
	}
	return true;
}

bool getMuleContactScores(PGconn* postgreSQL, string tableName,
													unordered_map <uint32_t, InfectionStats> &interestingIPs,
													InfectionStats &aggregateStats) {
	PGresult* result;
	uint32_t muleContactScore;
	string query = "SELECT \"ip\", COUNT(*) FROM \"" \
								 MULE_CONTACTS_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"ip\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		muleContactScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result,
																																index, 1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].
			muleContactScore = muleContactScore;
		aggregateStats.muleContactScore += muleContactScore;
	}
	return true;
}

bool getInfectedContactScores(PGconn* postgreSQL, string tableName,
															unordered_map <uint32_t, InfectionStats> &interestingIPs,
															InfectionStats &aggregateStats) {
	PGresult* result;
	uint32_t infectedContactScore;
	string query = "SELECT \"internalIP\", ROUND(SUM(\"weight\")) FROM \"" \
								 INFECTED_CONTACTS_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"internalIP\"";
	/*	
	 * Need to research the binary representation for doubles
	 * in PostgreSQL and use binary output for this.	
	 */
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												0);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		infectedContactScore = (uint32_t)strtoul(PQgetvalue(result, index, 1), NULL,
																						 10);
		interestingIPs[(uint32_t)strtoul(PQgetvalue(result, index, 0), NULL, 10)].
			infectedContactScore = infectedContactScore;
		aggregateStats.infectedContactScore += infectedContactScore;
	}
	return true;
}

bool getEvasiveTrafficScores(PGconn* postgreSQL, string tableName,
														 unordered_map <uint32_t, InfectionStats> &interestingIPs,
														 InfectionStats &aggregateStats) {
	PGresult* result;
	uint32_t evasiveTrafficScore;
	string query = "SELECT \"internalIP\", COUNT(*) FROM \"" \
								 EVASIVE_TRAFFIC_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"internalIP\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		evasiveTrafficScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result, index, 1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].
			evasiveTrafficScore = evasiveTrafficScore;
		aggregateStats.evasiveTrafficScore += evasiveTrafficScore;
	}
	return true;
}

bool getDarkSpaceSourceScores(PGconn* postgreSQL, string tableName,
															unordered_set <uint32_t> &liveIPs, unordered_map <uint32_t, InfectionStats> &interestingIPs,
															InfectionStats &aggregateStats) {
	PGresult* result;
	uint32_t ip, darkSpaceSourceScore;
	string query = "SELECT \"sourceIP\", COUNT(DISTINCT \"destinationIP\") " \
								 "FROM \"" \
								 DARK_SPACE_SOURCES_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"sourceIP\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		ip = be32toh(*(uint32_t*)PQgetvalue(result, index, 0));
		if (liveIPs.find(ip) != liveIPs.end()) {
			darkSpaceSourceScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result, index, 1));
			interestingIPs[ip].darkSpaceSourceScore = darkSpaceSourceScore;
			aggregateStats.darkSpaceSourceScore += darkSpaceSourceScore;
		}
	}
	return true;
}

bool getDarkSpaceTargetScores(PGconn* postgreSQL,
							  string tableName,
							  unordered_set <uint32_t> &,
							  unordered_map <uint32_t, InfectionStats>
							  		&interestingIPs,
							  InfectionStats &aggregateStats)
{
	PGresult* result;
	uint32_t darkSpaceTargetScore;
	string query = "SELECT \"internalIP\", COUNT(DISTINCT \"externalIP\") " \
								 "FROM \"" \
								 DARK_SPACE_TARGETS_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"internalIP\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		darkSpaceTargetScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result, index, 1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].
			darkSpaceTargetScore = darkSpaceTargetScore;
		aggregateStats.darkSpaceTargetScore += darkSpaceTargetScore;
	}
	return true;
}

bool getNonDNSTrafficScores(PGconn *postgreSQL, string tableName,
							unordered_map <uint32_t, InfectionStats> &interestingIPs,
							InfectionStats &aggregateStats)
{
	PGresult *result;
	uint32_t nonDNSTrafficScore;
	string query = "SELECT \"internalIP\", COUNT(*) FROM \"" \
								 NON_DNS_TRAFFIC_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"internalIP\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		nonDNSTrafficScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result,
																	  index,
																	  1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].
			nonDNSTrafficScore = nonDNSTrafficScore;
		aggregateStats.nonDNSTrafficScore += nonDNSTrafficScore;
	}
	return true;
}

bool getRebootScores(PGconn *postgreSQL, string tableName,
					 unordered_map <uint32_t, InfectionStats> &interestingIPs,
					 InfectionStats &aggregateStats)
{
	PGresult *result;
	uint32_t rebootScore;
	string query = "SELECT \"ip\", COUNT(*) FROM \"" \
								 REBOOTS_SCHEMA_NAME \
								 "\".\"" + tableName + "\" GROUP BY \"ip\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		rebootScore = (uint32_t)be64toh(*(uint64_t*)PQgetvalue(result, index, 1));
		interestingIPs[be32toh(*(uint32_t*)PQgetvalue(result, index, 0))].rebootScore = rebootScore;
		aggregateStats.rebootScore += rebootScore;
	}
	return true;
}

bool getWhiteList(unordered_map <string, unordered_set <uint32_t> > &whiteList,
					PGconn *postgreSQL,
					const char *date)
{
	PGresult *result;

	int exists(existsPGTable(postgreSQL, "RankWhiteList", date));

	if (exists == -1) {
		return false;
	}

	string table;
	if (exists) {
		table = date;
	}
	else {
		pair <bool, std::string> prevTablePair(getPreviousTable(postgreSQL,
																"RankWhiteList",
																date));
		if (prevTablePair.second == string("")) {
			return prevTablePair.first;
		}

		table = prevTablePair.second;
	}

	string query("SELECT \"ip\", \"rankName\" FROM \"RankWhiteList\".\"" +
				 table + "\"");
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL,
							NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		return false;
	}

	unordered_map <string, unordered_set <uint32_t> >::iterator mapIt;
	for (int i = 0; i < PQntuples(result); ++i) {
		mapIt = whiteList.find(PQgetvalue(result, i, 1));
		if (mapIt == whiteList.end()) {
			mapIt = whiteList.insert(make_pair(string(PQgetvalue(result, i, 1)), unordered_set <uint32_t>())).first;
		}
		mapIt->second.insert(be32toh(*(uint32_t*)PQgetvalue(result, i, 0)));
	}

	PQclear(result);
	return true;
}

bool calculateVirulences(PGconn *postgreSQL,
						 unordered_map <uint32_t, InfectionStats>
						 	&interestingIPs,
						 InfectionStats &aggregateStats, const char *date,
						 double steppingStoneWeight,
						 double muleContactWeight,
						 double infectedContactWeight,
						 double evasiveTrafficWeight,
						 double darkSpaceSourceWeight,
						 double nonDNSTrafficWeight,
						 double darkSpaceTargetWeight,
						 double rebootWeight,
						 double malwareSourceWeight,
						 double malwareTargetWeight,
						 const unordered_map<string, unordered_set <uint32_t> >
						 	&whiteList)
{
	ostringstream query;
	PGresult *result;
	uint32_t ip;
	uint16_t role;
	string roleName;
	unordered_map <uint32_t, InfectionStats>::iterator interestingIPItr;
	unordered_map <string, unordered_set <uint32_t> >::const_iterator whiteListItr;
	for (unordered_map <uint32_t, InfectionStats>::iterator ipItr = interestingIPs.begin();
			 ipItr != interestingIPs.end(); ++ipItr) {
	whiteListItr = whiteList.find("steppingStone");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.steppingStoneScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.steppingStoneScore * steppingStoneWeight) /
				aggregateStats.steppingStoneScore;
		}
	}
	whiteListItr = whiteList.find("muleContact");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.muleContactScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.muleContactScore * muleContactWeight) /
				aggregateStats.muleContactScore;
		}
	}
	whiteListItr = whiteList.find("infectedContact");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.infectedContactScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.infectedContactScore * infectedContactWeight) /
				aggregateStats.infectedContactScore;
		}
	}
	whiteListItr = whiteList.find("evasiveTraffic");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.evasiveTrafficScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.evasiveTrafficScore * evasiveTrafficWeight) /
				aggregateStats.evasiveTrafficScore;
		}
	}
	whiteListItr = whiteList.find("darkSpaceSource");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.darkSpaceSourceScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.darkSpaceSourceScore * darkSpaceSourceWeight) /
				aggregateStats.darkSpaceSourceScore;
		}
	}
	whiteListItr = whiteList.find("nonDNSTraffic");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.nonDNSTrafficScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.nonDNSTrafficScore * nonDNSTrafficWeight) /
				aggregateStats.nonDNSTrafficScore;
		}
	}
	whiteListItr = whiteList.find("darkSpaceTarget");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.darkSpaceTargetScore) {	
			ipItr -> second.currentVirulence +=
			(ipItr -> second.darkSpaceTargetScore * darkSpaceTargetWeight) /
				aggregateStats.darkSpaceTargetScore;
		}
	}
	whiteListItr = whiteList.find("reboot");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.rebootScore > 1) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.rebootScore * rebootWeight) /
				aggregateStats.rebootScore;
		}
	}
	whiteListItr = whiteList.find("malwareSource");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.malwareSourceScore) {
			ipItr -> second.currentVirulence +=
			(ipItr -> second.malwareSourceScore * malwareSourceWeight) /
				aggregateStats.malwareSourceScore;
		}
	}
	whiteListItr = whiteList.find("malwareTarget");
	if (whiteListItr == whiteList.end() || whiteListItr->second.find(ipItr->first) == whiteListItr->second.end()) {
		if (ipItr -> second.malwareTargetScore) {	 
			ipItr -> second.currentVirulence +=
			(ipItr -> second.malwareTargetScore * malwareTargetWeight) /
				aggregateStats.malwareTargetScore;
		}
	}
		aggregateStats.currentVirulence += ipItr -> second.currentVirulence;
	}
	for (unordered_map <uint32_t, InfectionStats>::iterator ipItr = interestingIPs.begin();
			 ipItr != interestingIPs.end(); ++ipItr) {
		ipItr -> second.currentVirulence /= aggregateStats.currentVirulence;
		ipItr -> second.minVirulence = ipItr -> second.currentVirulence;
		ipItr -> second.maxVirulence = ipItr -> second.currentVirulence;
	}
	/* Sets virulence of each host with a dangerous role to 1. */
	if (existsPGTable(postgreSQL, ROLES_SCHEMA_NAME, date) == 1) { 
		query << "SELECT \"ip\", \"role\" FROM \"" << ROLES_SCHEMA_NAME << "\".\""
					<< date << "\" WHERE ";
	for (size_t i(0); i < dangerousRolesCount - 1; ++i) {
		query << "\"role\" = '" << dangerousRoles[i] << "' OR ";
	}
	query << "\"role\" = '" << dangerousRoles[dangerousRolesCount - 1] << "'";
		result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
													NULL, 1);
		if (PQresultStatus(result) != PGRES_TUPLES_OK) {
			return false;
		}
		for (int row = 0; row < PQntuples(result); ++row) {
			ip = be32toh(*(uint32_t*)PQgetvalue(result, row, 0));
		role = be16toh(*(uint16_t*)PQgetvalue(result, row, 1));
		switch (role) {
		case SSH_BRUTE_FORCER:
		case TELNET_BRUTE_FORCER:
		case MICROSOFT_SQL_BRUTE_FORCER:
		case ORACLE_SQL_BRUTE_FORCER:
		case MYSQL_BRUTE_FORCER:
		case POSTGRESQL_BRUTE_FORCER:
			roleName = "bruteForcerBot";
			break;
		case DARK_SPACE_BOT:
			roleName = "darkSpaceBot";
			break;
		case SPAM_BOT:
			roleName = "spamBot";
			break;
		case SCAN_BOT:
			roleName = "scanBot";
			break;
		default:
			roleName = "NOT_A_ROLE";
			break;
		}
		whiteListItr = whiteList.find(roleName);
		if (whiteListItr == whiteList.end() || whiteListItr->second.find(ip) == whiteListItr->second.end()) {
				interestingIPItr = interestingIPs.find(ip);
				if (interestingIPItr == interestingIPs.end()) {
					interestingIPItr = interestingIPs.insert(make_pair(ip, InfectionStats())).first;
					interestingIPItr -> second.minVirulence = 1;
				}
				interestingIPItr -> second.currentVirulence = 1;
				interestingIPItr -> second.maxVirulence = 1;
		}
		}
	}
	return true;
}

bool getMinAndMaxVirulences(PGconn* postgreSQL,
														vector <string> &interestingIPDays,
														unordered_map <uint32_t, InfectionStats> &interestingIPs) {
	PGresult* result;
	uint32_t ip;
	double currentVirulence;
	for (size_t index = 0; index < interestingIPDays.size(); ++index) {
		string query = "SELECT \"ip\", \"currentVirulence\" FROM \"" \
									 INTERESTING_IPS_SCHEMA_NAME \
									 "\".\"" + interestingIPDays[index] + '"';
		/*	
		 * Need to research the binary representation for doubles
		 * in PostgreSQL and use binary output for this.	
		 */
		result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
													0);
		if (PQresultStatus(result) != PGRES_TUPLES_OK) {
			return false;
		}
		for (int index = 0; index < PQntuples(result); ++index) {
			ip = (uint32_t)strtol(PQgetvalue(result, index, 0), NULL, 10);
			currentVirulence = strtod(PQgetvalue(result, index, 1), NULL);
			if (interestingIPs.find(ip) != interestingIPs.end()) {
				if (currentVirulence < interestingIPs[ip].minVirulence) {
					interestingIPs[ip].minVirulence = currentVirulence;
				}
				if (currentVirulence > interestingIPs[ip].maxVirulence) {
					interestingIPs[ip].maxVirulence = currentVirulence;
				}
			}
		}
	}
	return true;
}

bool commitInterestingIPs(PGconn *postgreSQL, const char *date,
													size_t &flush_size,
													unordered_map <uint32_t, InfectionStats> &interestingIPs,
													vector <pair <uint32_t, uint32_t> > &localNetworks,
													bool reverseDNSLookups) {
	multimap <double, unordered_map <uint32_t, InfectionStats>::iterator> rankedInterestingIPs;
	vector <string> ptrRecords;
	size_t rank = 0;
	Clock clock("Inserted", "rows");
	PGBulkInserter pgBulkInserter(postgreSQL, INTERESTING_IPS_SCHEMA_NAME, date,
																flush_size, "%ud, %Vs, %ud, %ud, %ud, " \
																"%ud, %ud, %ud, %ud, %ud, %ud, %ud, %f, %f, " \
																"%f, %ud");
	if (!preparePGTable(postgreSQL, INTERESTING_IPS_SCHEMA_NAME, date,
											INTERESTING_IPS_TABLE_SCHEMA)) {
		return false;
	}
	for (unordered_map <uint32_t, InfectionStats>::iterator interestingIPItr = interestingIPs.begin();
			 interestingIPItr != interestingIPs.end(); ++interestingIPItr) {
		if (isInternal(interestingIPItr -> first, localNetworks)) {
			rankedInterestingIPs.insert(make_pair(interestingIPItr -> second.currentVirulence, interestingIPItr));
		}
	}
	cout << "Updating PostgreSQL database with interesting IPs" << endl;
	clock.start();
	for (multimap <double, unordered_map <uint32_t, InfectionStats>::iterator>::reverse_iterator rankedInterestingIPItr = rankedInterestingIPs.rbegin();
			 rankedInterestingIPItr != rankedInterestingIPs.rend(); ++rankedInterestingIPItr) {
		if (reverseDNSLookups) {
			ptrRecords = getPTRRecords(rankedInterestingIPItr -> second -> first);
		}
		if (!pgBulkInserter.insert(NULL, rankedInterestingIPItr -> second -> first,
															 (void*)&(ptrRecords),
															 rankedInterestingIPItr -> second -> second.steppingStoneScore,
															 rankedInterestingIPItr -> second -> second.muleContactScore,
															 rankedInterestingIPItr -> second -> second.infectedContactScore,
															 rankedInterestingIPItr -> second -> second.evasiveTrafficScore,
															 rankedInterestingIPItr -> second -> second.darkSpaceSourceScore,
															 rankedInterestingIPItr -> second -> second.darkSpaceTargetScore,
															 rankedInterestingIPItr -> second -> second.nonDNSTrafficScore,
															 rankedInterestingIPItr -> second -> second.rebootScore,
															 rankedInterestingIPItr -> second -> second.malwareSourceScore,
															 rankedInterestingIPItr -> second -> second.malwareTargetScore,
															 rankedInterestingIPItr -> second -> second.minVirulence,
															 rankedInterestingIPItr -> second -> second.maxVirulence,
															 rankedInterestingIPItr -> second -> second.currentVirulence,
															 ++rank)) {
			return false;
		}
		clock.incrementOperations();
	}
	if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
		return false;
	}
	clock.stop();
	return true;
}

bool updateInterestingIPDateIndex(PGconn *postgreSQL, string date,
																	unordered_map <uint32_t, InfectionStats> &interestingIPs) {
	unordered_set <uint32_t> oldInterestingIPs, indexedInterestingIPs;
	ostringstream query;
	PGresult *result = PQexecParams(postgreSQL,
																	"SELECT \"ip\" FROM \"Indexes\".\"" \
																	"interestingIPDates\"", 0, NULL, NULL, NULL,
																	NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		oldInterestingIPs.insert(be32toh(*(uint32_t*)PQgetvalue(result, index, 0)));
	}
	PQclear(result);
	query << "SELECT \"ip\" FROM \"Indexes\".\"interestingIPDates\" WHERE '"
				<< date << "' = ANY(\"dates\")";
	result = PQexecParams(postgreSQL, query.str().c_str(), 0, NULL, NULL, NULL,
												NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		indexedInterestingIPs.insert(be32toh(*(uint32_t*)PQgetvalue(result, index, 0)));
	}
	for (unordered_map <uint32_t, InfectionStats>::iterator interestingIPItr =
				 interestingIPs.begin(); interestingIPItr != interestingIPs.end();
			 ++interestingIPItr) {
		if (indexedInterestingIPs.find(interestingIPItr -> first) ==
					indexedInterestingIPs.end()) {
			if (oldInterestingIPs.find(interestingIPItr -> first) ==
						oldInterestingIPs.end()) {
				if (!insertPGRow(postgreSQL, "Indexes", "interestingIPDates", "%ud, %s",
												interestingIPItr -> first,
												string('{' + date + '}').c_str())) {
					return false;
				}
			}
			else {
				query.str("");
				query << "UPDATE \"Indexes\".\"interestingIPDates\" SET \"dates\" = "
							<< "array_append(\"dates\", '" << date << "') WHERE \"ip\" = '"
							<< interestingIPItr -> first << '\'';
				if (PQresultStatus(PQexec(postgreSQL, query.str().c_str())) !=
							PGRES_COMMAND_OK) {
					return false;
				}
			}
		}
	}
	return true;
}

void getLocalNetworks(vector <pair <uint32_t, uint32_t> > &localNetworks,
											const string &_localNetworks) {
	vector <string> __localNetworks = explodeString(_localNetworks, " ");
	for (size_t localNetwork = 0; localNetwork < __localNetworks.size();
			 ++localNetwork) {
		localNetworks.push_back(cidrToRange(__localNetworks[localNetwork]));
	}
}

bool getLiveIPs(PGconn *postgreSQL, std::string tableName,
								std::tr1::unordered_set <uint32_t> &liveIPs) {
	PGresult* result;
	std::string query = "SELECT \"ip\" FROM \"" \
											LIVE_IPS_SCHEMA_NAME \
											"\".\"" + tableName + '"';
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL,
												1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		return false;
	}
	for (int index = 0; index < PQntuples(result); ++index) {
		liveIPs.insert(be32toh(*(uint32_t*)PQgetvalue(result, index, 0)));
	}
	PQclear(result);
	return true;
}

			
int main(int argc, char* argv[]) {
	vector <pair <uint32_t, uint32_t> > localNetworks;
	unordered_set <uint32_t> liveIPs;
	vector <string> interestingIPDays;
	unordered_map <uint32_t, InfectionStats> interestingIPs;
	unordered_map <string, unordered_set <uint32_t> > whiteList;
	InfectionStats aggregateStats;
	if (argc < 2) {
		cerr << "usage: " << argv[0] << " date" << endl;
		return 1;
	}
	if (!isSQLDate(argv[1])) {
		cerr << argv[0] << ": malformed date" << endl;
		return 1;
	}

	configuration conf;
	if (!conf.load("/usr/local/etc/infer.conf")) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}
	
	PostgreSQLConnection pgConn;
	string str;
	configuration::error status;
	status = conf.get(str, "host", "postgresql");
	if (status == configuration::OK) {
		pgConn.host(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.host" << endl;
		return 1;
	}
	status = conf.get(str, "hostaddr", "postgresql");
	if (status == configuration::OK) {
		pgConn.hostaddr(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.hostaddr" << endl;
		return 1;
	}
	status = conf.get(str, "port", "postgresql");
	if (status == configuration::OK) {
		pgConn.port(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.port" << endl;
		return 1;
	}
	status = conf.get(str, "dbname", "postgresql");
	if (status == configuration::OK) {
		pgConn.dbname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.dbname" << endl;
		return 1;
	}
	status = conf.get(str, "user", "postgresql");
	if (status == configuration::OK) {
		pgConn.user(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.user" << endl;
		return 1;
	}
	status = conf.get(str, "password", "postgresql");
	if (status == configuration::OK) {
		pgConn.password(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.password" << endl;
		return 1;
	}
	status = conf.get(str, "connect-timeout", "postgresql");
	if (status == configuration::OK) {
		pgConn.connect_timeout(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.connect-timeout" << endl;
		return 1;
	}
	status = conf.get(str, "options", "postgresql");
	if (status == configuration::OK) {
		pgConn.options(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.options" << endl;
		return 1;
	}
	status = conf.get(str, "sslmode", "postgresql");
	if (status == configuration::OK) {
		pgConn.sslmode(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.sslmode" << endl;
		return 1;
	}
	status = conf.get(str, "requiressl", "postgresql");
	if (status == configuration::OK) {
		pgConn.requiressl(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.requiressl" << endl;
		return 1;
	}
	status = conf.get(str, "krbsrvname", "postgresql");
	if (status == configuration::OK) {
		pgConn.krbsrvname(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.krbsrvname" << endl;
		return 1;
	}
	status = conf.get(str, "gsslib", "postgresql");
	if (status == configuration::OK) {
		pgConn.gsslib(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.gsslib" << endl;
		return 1;
	}
	status = conf.get(str, "service", "postgresql");
	if (status == configuration::OK) {
		pgConn.service(str);
	}
	else if (status == configuration::BAD_DATA) {
		cerr << argv[0] << ": invalid postgresql.service" << endl;
		return 1;
	}

	size_t flush_size;
	if (conf.get(flush_size, "flush-size", "interesting_ips") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid flush-size" << endl;
		return 1;
	}

	string local_networks;
	if (conf.get(local_networks, "local-networks", "interesting_ips", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid local-networks" << endl;
		return 1;
	}

	bool reverse_dns_lookups;
	if (conf.get(reverse_dns_lookups, "reverse-dns-lookups", "interesting_ips") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid reverse-dns-lookups" << endl;
		return 1;
	}

	double steppingStoneWeight;
	if (conf.get(steppingStoneWeight, "steppingStoneWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid steppingStoneWeight" << endl;
		return 1;
	}

	double muleContactWeight;
	if (conf.get(muleContactWeight, "muleContactWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid muleContactWeight" << endl;
		return 1;
	}

	double infectedContactWeight;
	if (conf.get(infectedContactWeight, "infectedContactWeight",
				 "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid infectedContactWeight"
			 << endl;
		return 1;
	}

	double evasiveTrafficWeight;
	if (conf.get(evasiveTrafficWeight, "evasiveTrafficWeight",
				 "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid evasiveTrafficWeight" << endl;
		return 1;
	}

	double darkSpaceSourceWeight;
	if (conf.get(darkSpaceSourceWeight, "darkSpaceSourceWeight",
		"interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid darkSpaceSourceWeight"
			 << endl;
		return 1;
	}

	double nonDNSTrafficWeight;
	if (conf.get(nonDNSTrafficWeight, "nonDNSTrafficWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid nonDNSTrafficWeight" << endl;
		return 1;
	}

	double darkSpaceTargetWeight;
	if (conf.get(darkSpaceTargetWeight, "darkSpaceTargetWeight",
		"interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid darkSpaceTargetWeight"
			 << endl;
		return 1;
	}

	double rebootWeight;
	if (conf.get(rebootWeight, "rebootWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid rebootWeight" << endl;
		return 1;
	}

	double malwareSourceWeight;
	if (conf.get(malwareSourceWeight, "malwareSourceWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid malwareSourceWeight" << endl;
		return 1;
	}

	double malwareTargetWeight;
	if (conf.get(malwareTargetWeight, "malwareTargetWeight", "interesting_ips")
		!= configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid malwareTargetWeight" << endl;
		return 1;
	}


	if (!pgConn.open()) {
		cerr << argv[0] << ": unable to open PostgreSQL connection" << endl
			 << pgConn.error() << endl;
		return 1;
	}
	PGconn *postgreSQL(pgConn.connection());
	if (PQstatus(postgreSQL) != CONNECTION_OK) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	getLocalNetworks(localNetworks, local_networks);
	if (!getLiveIPs(postgreSQL, argv[1], liveIPs)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!getInterestingIPDays(postgreSQL, argv[1], interestingIPDays)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!preparePGTable(postgreSQL, PROCESS_STATS_SCHEMA_NAME, argv[1],
											PROCESS_STATS_TABLE_SCHEMA, "processName",
											PROCESS_NAME)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME, 0, 0)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, STEPPING_STONES_SCHEMA_NAME, argv[1]) == 1 &&
			!getSteppingStoneScores(postgreSQL, argv[1], interestingIPs,
															aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, MULE_CONTACTS_SCHEMA_NAME, argv[1]) == 1 &&
			!getMuleContactScores(postgreSQL, argv[1], interestingIPs,
														aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, INFECTED_CONTACTS_SCHEMA_NAME, argv[1]) == 1 &&
			!getInfectedContactScores(postgreSQL, argv[1], interestingIPs,
																aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, EVASIVE_TRAFFIC_SCHEMA_NAME, argv[1]) == 1 &&
			!getEvasiveTrafficScores(postgreSQL, argv[1], interestingIPs,
															 aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, DARK_SPACE_SOURCES_SCHEMA_NAME, argv[1]) == 1 &&
			!getDarkSpaceSourceScores(postgreSQL, argv[1], liveIPs, interestingIPs,
																aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, DARK_SPACE_TARGETS_SCHEMA_NAME, argv[1]) == 1 &&
			!getDarkSpaceTargetScores(postgreSQL, argv[1], liveIPs, interestingIPs,
																aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, NON_DNS_TRAFFIC_SCHEMA_NAME, argv[1]) == 1 &&
			!getNonDNSTrafficScores(postgreSQL, argv[1], interestingIPs,
															aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (existsPGTable(postgreSQL, REBOOTS_SCHEMA_NAME, argv[1]) == 1 &&
			!getRebootScores(postgreSQL, argv[1], interestingIPs, aggregateStats)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!getWhiteList(whiteList, postgreSQL, argv[1])) {
		cerr << "Error getting white list from postgresql." << endl
		 << PQerrorMessage(postgreSQL);
	return 1;
	}
	if (!calculateVirulences(postgreSQL, interestingIPs, aggregateStats,
													 argv[1],
													 steppingStoneWeight,
													 muleContactWeight,
													 infectedContactWeight,
													 evasiveTrafficWeight,
													 darkSpaceSourceWeight,
													 nonDNSTrafficWeight,
													 darkSpaceTargetWeight,
													 rebootWeight,
													 malwareSourceWeight,
													 malwareTargetWeight,
													 whiteList)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!getMinAndMaxVirulences(postgreSQL, interestingIPDays, interestingIPs)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!commitInterestingIPs(postgreSQL, argv[1], flush_size, interestingIPs,
														localNetworks,
														reverse_dns_lookups)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!updateInterestingIPDateIndex(postgreSQL, argv[1], interestingIPs)) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	if (!updateProcessStats(postgreSQL, argv[1], PROCESS_NAME, 100,
													interestingIPs.size())) {
		cerr << PQerrorMessage(postgreSQL);
		return 1;
	}
	return 0;
}
