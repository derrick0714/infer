#include <string>
#include <tr1/unordered_map>

#include "PostgreSQLBulkWriter.hpp"
#include "modules.hpp"
#include "hostPair.hpp"

#include "host_as_exposure_datum.hpp"

using namespace std;
using namespace tr1;

SharedState *_shared_state_p;

// stats collection
struct as_stat_t {
	as_stat_t()
		:internal_hosts(),
		 ingress_bytes(0),
		 egress_bytes(0)
	{}

	// host -> (ingress_bytes, egress_bytes)
	unordered_map<uint32_t, pair<size_t, size_t> > internal_hosts;
	size_t ingress_bytes;
	size_t egress_bytes;
};

map<uint16_t, as_stat_t> as_stats;

// configuration options
string _host_as_exposure_schema;

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		_shared_state_p = &sharedState;

		if (moduleState.conf.get(_host_as_exposure_schema,
								 "host-as-exposure-schema",
								 "analysis_host_exposure")
				!= configuration::OK)
		{
			cerr << "analysis_host_exposure: "
						"missing or invalid host-as-exposure-schema"
				 << endl;
			return false;
		}

		return true;
	}

	void aggregate(FlowStats *flow_stats, size_t &) {
		static uint16_t asn;

		if (isInternal(flow_stats->sourceIP(),
					   *(_shared_state_p->localNetworks)))
		{
			if (isInternal(flow_stats->destinationIP(),
						   *(_shared_state_p->localNetworks)))
			{
				return;
			}

			asn = _shared_state_p->
					ipInformation->
						getASN(flow_stats->destinationIP());
			as_stats[asn].internal_hosts[flow_stats->sourceIP()].second
				+= flow_stats->numBytes();
			as_stats[asn].egress_bytes += flow_stats->numBytes();
		}
		else {
			if (!isInternal(flow_stats->destinationIP(),
							*(_shared_state_p->localNetworks)))
			{
				return;
			}

			asn = _shared_state_p->
					ipInformation->
						getASN(flow_stats->sourceIP());
			as_stats[asn].internal_hosts[flow_stats->destinationIP()].first
				+= flow_stats->numBytes();
			as_stats[asn].ingress_bytes += flow_stats->numBytes();
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &,
			   const char *date)
	{
		PostgreSQLBulkWriter<host_as_exposure_datum>
			host_as_writer(pg_conn,
				   _host_as_exposure_schema,
				   date,
				   true);

		host_as_exposure_datum host_as_datum;
		int rows;
		for (map<uint16_t, as_stat_t>::const_iterator i(as_stats.begin());
			 i != as_stats.end();
			 ++i)
		{
			host_as_datum.asn = i->first;
			for (unordered_map<uint32_t, pair<size_t, size_t> >::const_iterator
					j(i->second.internal_hosts.begin());
				 j != i->second.internal_hosts.end();
				 ++j)
			{
				host_as_datum.host = j->first;
				host_as_datum.ingress_bytes = j->second.first;
				host_as_datum.egress_bytes = j->second.second;

				if (!host_as_writer.write(host_as_datum)) {
					cerr << "host_exposure: write failed!" << endl;
					return 1;
				}

				++rows;
			}
		}

		host_as_writer.flush();
		host_as_writer.close();

		return rows;
	}
}
