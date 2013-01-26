#include <iostream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "PostgreSQLBulkWriter.hpp"
#include "PostgreSQLWriter.hpp"

namespace std {
namespace tr1 {

template<>
struct hash<pair<uint32_t, uint32_t> >
{
	std::tr1::hash<uint64_t> uint64_hasher;

	std::size_t operator()(const std::pair<uint32_t, uint32_t> &p) const {
		return uint64_hasher(*(reinterpret_cast<const uint64_t *>(&p)));
	}
};

}
}

using namespace std;
using namespace tr1;

typedef pair<uint32_t, uint32_t> host_ip_pair;

vector <pair <uint32_t, uint32_t> > *localNetworks;
unordered_map <uint32_t, string> *liveIPs;

unordered_map<uint32_t, uint32_t> internal_ip_index;
unordered_map<uint32_t, uint32_t> external_ip_index;
unordered_set<host_ip_pair> host_ip_pairs;

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &) {
		localNetworks = sharedState.localNetworks;
		liveIPs = sharedState.liveIPs;
		return true;
	}

	void aggregate(const FlowStats *flowStats, size_t) {
		if (liveIPs->find(flowStats->sourceIP()) != liveIPs->end() &&
				!isInternal(flowStats->destinationIP(), *localNetworks))
		{
			if (internal_ip_index.find(flowStats->sourceIP()) ==
					internal_ip_index.end())
			{
				internal_ip_index.insert(std::pair<uint32_t, uint32_t>(
											flowStats->sourceIP(),
											internal_ip_index.size()));
			}
			if (external_ip_index.find(flowStats->sourceIP()) ==
					external_ip_index.end())
			{
				external_ip_index.insert(std::pair<uint32_t, uint32_t>(
											flowStats->destinationIP(),
											external_ip_index.size()));
			}

			host_ip_pairs.insert(host_ip_pair(flowStats->sourceIP(),
											  flowStats->destinationIP()));
		}
		else if (liveIPs -> find(flowStats->destinationIP())
						!= liveIPs -> end() &&
					!isInternal(flowStats -> sourceIP(), *localNetworks))
		{
			if (internal_ip_index.find(flowStats->destinationIP()) ==
					internal_ip_index.end())
			{
				internal_ip_index.insert(std::pair<uint32_t, uint32_t>(
											flowStats->destinationIP(),
											internal_ip_index.size()));
			}
			if (external_ip_index.find(flowStats->sourceIP()) ==
					external_ip_index.end())
			{
				external_ip_index.insert(std::pair<uint32_t, uint32_t>(
											flowStats->sourceIP(),
											external_ip_index.size()));
			}

			host_ip_pairs.insert(host_ip_pair(flowStats->destinationIP(),
											  flowStats->sourceIP()));
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &,
			   const char *date)
	{
		string host_index_schema("HostIndexes");
		string host_index_pair_schema("HostIndexPairs");
		string stats_schema("Stats");
		string network_stats_table("network");
		size_t buffer_count(500000);

		Clock clock("Inserted", "rows");
		cerr << "Updating PostgreSQL database with host pairs" << endl;
		clock.start();
		PostgreSQLBulkWriter<host_ip_index> index_writer(pg_conn,
														 host_index_schema,
														 date,
														 true);

		if (!index_writer) {
			cerr << "hostPairs: index_writer initialization failed: " << endl;
			cerr << "\t" << index_writer.error() << endl;
			return -1;
		}

		size_t rows(0);
		for (unordered_map<uint32_t, uint32_t>::iterator i(
				internal_ip_index.begin());
			 i != internal_ip_index.end();
			 ++i)
		{
			if (!index_writer.write(host_ip_index(i->first, i->second))) {
				cerr << "hostPairs: unable to write internal host index!" << endl;
				return -1;
			}
			clock.incrementOperations();
			++rows;
			if (rows % buffer_count == 0) {
				if (!index_writer.flush()) {
					cerr << "hostPairs: index_writer.flush() failed!" << endl;
					return -1;
				}
			}
		}
		cerr << "internal host indexes written:  " << rows << endl;
		size_t internal_rows(rows);

		for (unordered_map<uint32_t, uint32_t>::iterator i(
				external_ip_index.begin());
			 i != external_ip_index.end();
			 ++i)
		{
			if (!index_writer.write(
					host_ip_index(i->first,
								  internal_ip_index.size() + i->second)))
			{
				cerr << "hostPairs: unable to write external host index!" << endl;
				return -1;
			}
			clock.incrementOperations();
			++rows;
			if (rows % buffer_count == 0) {
				if (!index_writer.flush()) {
					cerr << "hostPairs: index_writer.flush() failed!" << endl;
					return -1;
				}
			}
		}
		size_t external_rows(rows - internal_rows);
		cerr << "external host indexes written:  " << external_rows << endl;

		if (!index_writer.close()) {
			cerr << "hostPairs: index_writer.close() failed: " << endl;
			cerr << "\t" << index_writer.error() << endl;
			return -1;
		}

		string query("ALTER TABLE \"" + host_index_schema + "\".\"" + date + "\" ADD PRIMARY KEY(host);");
		PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
		if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
			if (result != NULL) {
				PQclear(result);
			}
			return false;
		}
		PQclear(result);

		query = ("ALTER TABLE \"" + host_index_schema + "\".\"" + date + "\" ADD UNIQUE(id);");
		result = PQexec(pg_conn.connection(), query.c_str());
		if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
			if (result != NULL) {
				PQclear(result);
			}
			return false;
		}
		PQclear(result);

		cerr << "total host indexes written:     " << rows << endl;

		PostgreSQLBulkWriter<host_index_pair> index_pair_writer(
															pg_conn,
															host_index_pair_schema,
															date,
															true);

		if (!index_pair_writer) {
			cerr << "hostPairs: index_pair_writer initialization failed: "
				 << endl;
			cerr << "\t" << index_pair_writer.error() << endl;
			return -1;
		}

		rows = 0;
		for (unordered_set<host_ip_pair>::const_iterator i(
				host_ip_pairs.begin());
			 i != host_ip_pairs.end();
			 ++i)
		{
			if (!index_pair_writer.write(
					host_index_pair(
						internal_ip_index[i->first],
						external_ip_index[i->second] + internal_ip_index.size())))
			{
				cerr << "hostPairs: unable to write host index pair!" << endl;
				return -1;
			}

			clock.incrementOperations();
			++rows;
			if (rows % buffer_count == 0) {
				if (!index_pair_writer.flush()) {
					cerr << "hostPairs: index_pair_writer.flush() failed!"
						 << endl;
					return -1;
				}
			}
		}
		if (!index_pair_writer.close()) {
			cerr << "hostPairs: index_pair_writer.close() failed: " << endl;
			cerr << "\t" << index_pair_writer.error() << endl;
			return -1;
		}

		cerr << "total host index pairs written: " << rows << endl;

		query = ("DELETE FROM \"" + stats_schema + "\".\"" + network_stats_table + "\" WHERE date = '" + date + "';");
		result = PQexec(pg_conn.connection(), query.c_str());
		if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << "Error: PostgreSQL: " << PQerrorMessage(pg_conn.connection());
			if (result != NULL) {
				PQclear(result);
			}
			return false;
		}
		PQclear(result);

		network_stats net_stats(date, internal_rows, external_rows);
		PostgreSQLWriter<network_stats> stats_writer(pg_conn,
													 stats_schema,
													 network_stats_table);

		if (!stats_writer) {
			cerr << "hostPairs: stats_writer initialization failed: "
				 << endl;
			cerr << "\t" << stats_writer.error() << endl;
			return -1;
		}

		if (!stats_writer.write(net_stats)) {
			cerr << "hostPairs: unable to write network stats!" << endl;
			return -1;
		}
		clock.incrementOperations();

		clock.stop();

		internal_ip_index.clear();
		external_ip_index.clear();
		host_ip_pairs.clear();

		return clock.operations();
	}
}
