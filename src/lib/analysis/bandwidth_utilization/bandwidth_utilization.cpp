#include <string>

#include "PostgreSQLBulkWriter.hpp"
#include "modules.hpp"
#include "hostPair.hpp"

#include "bandwidth_utilization_datum.hpp"

using namespace std;
using namespace tr1;

SharedState *_shared_state_p;

// stats collection
map<uint32_t, double> _ingress_bw_utilization;
map<uint32_t, double> _egress_bw_utilization;

// configuration options
size_t _interval_length;
string _stats_schema;
string _bandwidth_utilization_stats_table;

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		_shared_state_p = &sharedState;

		if (moduleState.conf.get(_interval_length,
								 "interval-length",
								 "analysis_bandwidth_utilization")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_utilization: "
						"missing or invalid interval-length"
				 << endl;
			return false;
		}

		if (moduleState.conf.get(_stats_schema,
								 "stats-schema",
								 "analysis_bandwidth_utilization")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_utilization: "
						"missing or invalid stats-schema"
				 << endl;
			return false;
		}

		if (moduleState.conf.get(_bandwidth_utilization_stats_table,
								 "bandwidth-utilization-stats-table",
								 "analysis_bandwidth_utilization")
				!= configuration::OK)
		{
			cerr << "analysis_bandwidth_utilization: "
						"missing or invalid bandwidth-utilization-stats-table"
				 << endl;
			return false;
		}

		return true;
	}

	void aggregate(FlowStats *flowStats, size_t &hour) {
		static uint32_t begin;
		static uint32_t end;
		static double bytes;
		static map<uint32_t, double> *bw;

		begin = flowStats->startTime().seconds() -
					flowStats->startTime().seconds() % _interval_length;
		end = flowStats->endTime().seconds() -
					(flowStats->endTime().seconds() % _interval_length) +
					_interval_length;
		bytes = static_cast<double>(flowStats->numBytes()) / 
					((end - begin) / _interval_length);

		if (isInternal(flowStats->sourceIP(),
					   *(_shared_state_p->localNetworks)))
		{
			if (isInternal(flowStats->destinationIP(),
						   *(_shared_state_p->localNetworks)))
			{
				return;
			}

			bw = &_egress_bw_utilization;
		}
		else {
			if (!isInternal(flowStats->destinationIP(),
							*(_shared_state_p->localNetworks)))
			{
				return;
			}

			bw = &_ingress_bw_utilization;
		}

		while (begin < end) {
			(*bw)[begin] += bytes;
			begin += _interval_length;
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &flushSize,
			   const char *date)
	{
		struct tm tm_;
		if (strptime(date, "%Y-%m-%d", &tm_) == NULL) {
			cerr << "analysis_bandwidth_utilization: invalid date: " << date
				 << endl;
			return -1;
		}

		struct tm tm_begin, tm_end;
		memset(&tm_begin, 0, sizeof(tm_begin));
		memset(&tm_end, 0, sizeof(tm_end));

		tm_begin.tm_isdst = tm_end.tm_isdst = -1;
		tm_begin.tm_year = tm_end.tm_year = tm_.tm_year;
		tm_begin.tm_mon = tm_end.tm_mon = tm_.tm_mon;
		tm_begin.tm_mday = tm_end.tm_mday = tm_.tm_mday;
		tm_begin.tm_hour = 0;
		tm_end.tm_hour = 24;

		uint32_t begin(mktime(&tm_begin));
		uint32_t end(mktime(&tm_end));

		string query("DELETE FROM \"" + _stats_schema + "\".\"" +
					 _bandwidth_utilization_stats_table +
					 "\" WHERE interval_start >= '" +
					 boost::lexical_cast<string>(begin) +
					 "' AND interval_start < '" +
					 boost::lexical_cast<string>(end) + "';");
		PGresult *result(PQexec(pg_conn.connection(), query.c_str()));
		if (result == NULL || PQresultStatus(result) != PGRES_COMMAND_OK) {
			cerr << "Error: PostgreSQL: "
				 << PQerrorMessage(pg_conn.connection());
			if (result != NULL) {
				PQclear(result);
			}
			return -1;
		}
		PQclear(result);

		PostgreSQLBulkWriter<bandwidth_utilization_datum> 
			writer(pg_conn,
				   _stats_schema,
				   _bandwidth_utilization_stats_table);

		bandwidth_utilization_datum datum;
		int rows;
		for (uint32_t i(begin); i < end; i += _interval_length) {
			datum.time_seconds = i;
			datum.ingress_bytes_per_second =
				_ingress_bw_utilization[i] / _interval_length;
			datum.egress_bytes_per_second =
				_egress_bw_utilization[i] / _interval_length;

			writer.write(datum);

			/*
			cout << scientific << i << '\t' << fixed << setprecision(3)
				 << _ingress_bw_utilization[i] / _interval_length << '\t'
				 << _egress_bw_utilization[i] / _interval_length
				 << endl;
			*/
			++rows;
		}

		writer.flush();
		writer.close();

		return rows;
	}
}
