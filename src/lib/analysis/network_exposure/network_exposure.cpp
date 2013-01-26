#include <string>
#include <tr1/unordered_map>

#include "PostgreSQLBulkWriter.hpp"
#include "modules.hpp"
#include "hostPair.hpp"

#include "as_exposure_datum.hpp"

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

	unordered_set<uint32_t> internal_hosts;
	size_t ingress_bytes;
	size_t egress_bytes;
};

map<uint16_t, as_stat_t> as_stats;

// configuration options
string _as_exposure_schema;

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		_shared_state_p = &sharedState;

		if (moduleState.conf.get(_as_exposure_schema,
								 "as-exposure-schema",
								 "analysis_network_exposure")
				!= configuration::OK)
		{
			cerr << "analysis_network_exposure: "
						"missing or invalid as-exposure-schema"
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
			as_stats[asn].internal_hosts.insert(flow_stats->sourceIP());
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
			as_stats[asn].internal_hosts.insert(flow_stats->destinationIP());
			as_stats[asn].ingress_bytes += flow_stats->numBytes();
		}
	}

	int commit(PostgreSQLConnection &pg_conn,
			   size_t &,
			   const char *date)
	{
		PostgreSQLBulkWriter<as_exposure_datum>
			as_writer(pg_conn,
				   _as_exposure_schema,
				   date,
				   true);

		as_exposure_datum as_datum;
		int rows;
		for (map<uint16_t, as_stat_t>::const_iterator i(as_stats.begin());
			 i != as_stats.end();
			 ++i)
		{
			as_datum.asn = i->first;
			as_datum.internal_hosts_contacted = i->second.internal_hosts.size();
			as_datum.ingress_bytes = i->second.ingress_bytes;
			as_datum.egress_bytes = i->second.egress_bytes;

			as_writer.write(as_datum);

			++rows;
		}

		as_writer.flush();
		as_writer.close();

		return rows;
	}
}
