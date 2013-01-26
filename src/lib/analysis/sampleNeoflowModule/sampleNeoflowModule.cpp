#include <tr1/unordered_map>
#include <tr1/unordered_set>

#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"

using namespace std;
using namespace tr1;

/*
 * Pointers to various state maintained by the main executable. Their use and
 * presence here is optional.
 */
/* Live IPs on the network. */
unordered_map <uint32_t, string> *liveIPs;
/*
 * TCP initiators. See header file for details. It should not be necessary to
 * call aggregate() on this class for any stage-2 modules because the
 * commChannels module aggregates all TCP flows and is in stage 1.
 */
ConnectionInitiators *connectionInitiators;
/* IP ASN and country information. See header file for details. */
IPInformation *ipInformation;

/*
 * State specific to the module should be declared here. Any data structures
 * whose keys will be flows (tuples of the Layer-4 protocol and IP and port
 * pairs) should make use of the OneWayHostPair or TwoWayHostPair classes for
 * keys. The makeOneWayHostPair() and makeTwoWayHostPair() functions should be
 * used in aggregate() to convert FlowStats structures to the classes easily.
*/

extern "C" {
  /*
   * This function is called once, before the module is given any neoflow
   * records. Returns true upon success, or false upon failure. The PGconn and
   * date (in YYYY-MM-DD format) variables should be used in case it's necessary
   * to get something from the database. The conf variable contains any
   * configuration options from the module's configuration file, which always
   * takes the module's name and a ".conf" suffix. Options are stored as
   * std::string types and are indexed by their names in the configuration file
   * (conf.options["optionName"]). The rest of the variables are pointers to
   * state maintained by the main executable, and should be assigned to the
   * global pointers to the state.
   */
  bool initialize(SharedState &sharedState, ModuleState &moduleState) {
    sharedState.liveIPs = liveIPs;
    sharedState.connectionInitiators = connectionInitiators;
    sharedState.ipInformation = ipInformation;
    return true;
  }

  /*
   * This function is called for every neoflow record. The hour variable is the
   * hour of the day (0 through 23) the record belongs to. Flows with the same
   * flow ID (protocol, source IP, destination IP, source port, and destination
   * port tuple) and guaranteed to appear in chronological order, whereas flows
   * with different flow IDs aren't, and likely won't.
   */
  void aggregate(FlowStats *flowStats, size_t &hour) {
  }

  /*
   * This function is called once, after aggregate() has been called for each
   * neoflow record. Typical things to do here are to prepare the appropriate
   * SQL table using preparePGTable(), commit any information gathered using the
   * PGBulkInserter class, and clear any state kept by the module. Returns the
   * number of SQL rows inserted upon success, or -1 upon failure.
   */
  int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
  }
}
