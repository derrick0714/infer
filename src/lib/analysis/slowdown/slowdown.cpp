#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <numeric>
#include <stdlib.h>
#include <algorithm>
#include <sys/endian.h>

#include "timeStamp.h"
#include "sqlTime.h"
#include "modules.hpp"
#include "postgreSQL.h"
#include "hostPair.hpp"
#include "clock.hpp"
#include "smoothing.h"
#include "forecast.h"
#include "MiscHelpers.hpp"

//1 bin ahead(5 mins)
#define FORECAST_AHEAD 1

#define SLOWDOWN_TABLE_SCHEMA "\"internalIP\" uint32 NOT NULL, \
						\"firstBinTime\" uint32 NOT NULL, \
						\"lastBinTime\" uint32 NOT NULL, \
						\"dayStartTime\" uint32 NOT NULL, \
						\"alertStartTime\" uint32 NOT NULL, \
						\"alertEndTime\" uint32 NOT NULL, \
						\"lastBinValue\" double precision NOT NULL, \
						\"deltaSum\" double precision NOT NULL, \
						\"smoothedObs\" double precision[] NOT NULL, \
						\"trendSmoothedObs\" double precision[] NOT NULL, \
						\"seasonalSmoothedObs\" double precision[] NOT NULL, \
						\"delta\" double precision[] NOT NULL, \
						\"forecast\" double precision[] NOT NULL, \
						\"deviation\" double precision[] NOT NULL, \
						\"confidence\" double precision[] NOT NULL, \
						\"connections\" uint32[] NOT NULL, \
						PRIMARY KEY (\"internalIP\")"

using namespace std;
using namespace tr1;

uint32_t dataPoints=0, copiedBins=0, filledBins=0;


/*
 * Pointers to various state maintained by the main executable. Their use and
 * presence here is optional.
 */
/* Live IPs on the network. */
unordered_map <uint32_t, string> *liveIPs;
/*
 * Responses to DNS A record requests. The keys are internal IPs and the values
 * are IPs that were returned in DNS A record requests.
 */
unordered_map <uint32_t, unordered_set <uint32_t> > *dnsResponses;
/*
 * TCP initiators. See header file for details. It should not be necessary to
 * call aggregate() on this class for any stage-2 modules because the
 * commChannels module aggregates all TCP flows and is in stage 1.
 */
ConnectionInitiators *connectionInitiators;

/* IP ASN and country information. See header file for details. */
IPInformation *ipInformation;

vector <pair <uint32_t, uint32_t> > *localNetworks;

/*
 * State specific to the module should be declared here. Any data structures
 * whose keys will be flows (tuples of the Layer-4 protocol and IP and port
 * pairs) should make use of the OneWayHostPair or TwoWayHostPair classes for
 * keys. The makeOneWayHostPair() and makeTwoWayHostPair() functions should be
 * used in aggregate() to convert FlowStats structures to the classes easily.
*/

/*brief class to store flowID data */
class HandshakeTimings{
public:
				int8_t initiator;
				TimeStamp firstSYNTime;
				TimeStamp firstACKTime;
				TimeStamp firstSYNACKTime;
				HandshakeTimings();
};
inline HandshakeTimings::HandshakeTimings(){
				initiator = TEMPORARILY_UNKNOWN_INITIATOR;
				firstSYNTime.set(0,0);
				firstACKTime.set(0,0);
				firstSYNACKTime.set(0,0);
}

/*container to store flowID data*/
map <TwoWayHostPair, HandshakeTimings, std::less<TwoWayHostPair> > hostStats;
map <TwoWayHostPair, HandshakeTimings>::iterator hostStatsItr;

/*container to store observed timings. Cleared at the end of each hour*/
IPResponseTimes ipResponseTimes;
map <uint32_t, ResponseTimings>::iterator ipResponseTimesItr;

/*container to store smoothed Timings for bins*/
map <uint32_t, SmoothedReadings> smoothedValues;

/*container to store the last seen bin time for each IP to enable visibility across hours*/
PrevBinTime prevBinTime;

TwoWayHostPair hostPair;
SmoothedReadings smoothedReadings;

size_t hourTrack=0, slowdownCount=0, tcpConnectionTimeout=0;
uint32_t hourStartTime, dayStartTime;
double delta=0;
pair <bool, std::string> previousTable;
pair <bool, std::string> prePreviousTable;

ForecastParameters forecastParameters;
map <uint32_t, ForecastParameters> forecastParam;
map <uint32_t, ForecastParameters>::iterator forecastParamItr;

map <uint32_t, vector<double> > prevObservedObs;
map <uint32_t, vector<double> >::iterator prevObservedObsItr;

double zero() { return 0; }

/* ///\brief function to fetch the last bin readings from previous day
 * ///\param postgreSQL instance of PostgreSQL which specifies DB paramters
 * ///\param prevBinTime specifies map to be populated with previous days last reading
 * ///\param date specifies closest previous avaliable table
 */
void fetchPrevBinData(PGconn *postgreSQL,
					  PrevBinTime &prevBinTime,
					  const string &)
{
	PGresult *result;
	string query("SELECT \"internalIP\", \"lastBinTime\", \"lastBinValue\" from \"Slowdown\".\"");
	query += previousTable.second + "\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		cerr<<PQerrorMessage(postgreSQL)<<endl;
	}
	else{
		uint32_t ip, lBinTime;
		uint64_t _lBinValue;
		double lBinValue;
		for (int i = 0; i < PQntuples(result); ++i) {
			ip=ntohl(*(uint32_t*)PQgetvalue(result, i, 0));
			lBinTime=ntohl(*(uint32_t*)PQgetvalue(result, i, 1));;
			_lBinValue = be64toh(*(uint64_t *) PQgetvalue(result, i, 2));
			memmove(&lBinValue, &_lBinValue, 8);
			prevBinTime.insert(make_pair(ip, make_pair(lBinTime, lBinValue)));
		}
		PQclear(result);
	}
}

/* ///\brief function to fetch forecast params from past data
 * ///\param postgreSQL instance of PostgreSQL which specifies DB paramters
 * ///\param prevData specifies closest previous table
 * ///\param prePrevData specifies closest table before previous table
 */
void fetchPrevConsolidatedForecastData(PGconn *postgreSQL, const string &prevDate, const string &prePrevDate) {
	PGresult *result;
	size_t offset = BINS_IN_DAY - FORECAST_AHEAD;
string query("SELECT \"internalIP\", \"smoothedObs\", \"trendSmoothedObs\", \"seasonalSmoothedObs\", array_upper(\"smoothedObs\", 1), array_upper(\"trendSmoothedObs\", 1), array_upper(\"seasonalSmoothedObs\", 1), \"deltaSum\", \"deviation\",	array_upper(\"deviation\", 1), \"connections\",	array_upper(\"connections\", 1) from \"Slowdown\".\"");
	query += prevDate + "\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		cerr<<PQerrorMessage(postgreSQL)<<endl;
	}
	else{
		uint32_t ip, smoothedObsLen, trendSmoothedObsLen, seasonalSmoothedObsLen, deviationObsLen, connectionsLen;
		uint64_t _deltaSum;
		uint64_t _smoothedObs;
		uint64_t _trendSmoothedObs;
		uint64_t _seasonalSmoothedObs;
		uint64_t _deviationObs;
		char *smoothedObsArr;
		char *trendSmoothedObsArr;
		char *seasonalSmoothedObsArr;
		char *deviationObsArr;
		char *connectionsArr;
		double smoothedObs;
		double trendSmoothedObs;
		double seasonalSmoothedObs;
		double deviationObs;
		for (int i = 0; i < PQntuples(result); ++i) {
			ip=ntohl(*(uint32_t*)PQgetvalue(result, i, 0));
			_deltaSum = be64toh(*(uint64_t *) PQgetvalue(result, i, 7));
			memmove(&forecastParameters.deltaSum, &_deltaSum, 8);
			smoothedObsArr = PQgetvalue(result, i, 1);
			trendSmoothedObsArr = PQgetvalue(result, i, 2);
			seasonalSmoothedObsArr = PQgetvalue(result, i, 3);
			deviationObsArr = PQgetvalue(result, i, 8);
			connectionsArr = PQgetvalue(result, i, 10);
			smoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 4));
			trendSmoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 5));
			if( trendSmoothedObsLen != BINS_IN_DAY )
				trendSmoothedObsLen =	0;
			seasonalSmoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 6));
			deviationObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 9));
			connectionsLen = ntohl(*(uint32_t *) PQgetvalue(result, i, 11));
			if( deviationObsLen != BINS_IN_DAY )
				deviationObsLen =	0;
			for(size_t j = 0; j < smoothedObsLen; ++j) {
				_smoothedObs = be64toh(*(uint64_t *)((smoothedObsArr + 20) + (j * 12) + 4));
				memmove(&smoothedObs, &_smoothedObs, 8);
				forecastParameters.prevSmoothedObs.push_back(smoothedObs);
			}
			for(size_t k = 0; k < trendSmoothedObsLen; ++k) {
				_trendSmoothedObs = be64toh(*(uint64_t *)((trendSmoothedObsArr + 20) + (k * 12) + 4));
				memmove(&trendSmoothedObs, &_trendSmoothedObs, 8);
				forecastParameters.prevTrendSmoothedObs.push_back(trendSmoothedObs);
			}
			for(size_t l = 0; l < seasonalSmoothedObsLen; ++l) {
				_seasonalSmoothedObs = be64toh(*(uint64_t *)((seasonalSmoothedObsArr + 20) + (l * 12) + 4));
				memmove(&seasonalSmoothedObs, &_seasonalSmoothedObs, 8);
				forecastParameters.prevSeasonalSmoothedObs.push_back(seasonalSmoothedObs);
			}
			for(size_t m = 0; m < deviationObsLen; ++m) {
				_deviationObs = be64toh(*(uint64_t *)((deviationObsArr + 20) + (m * 12) + 4));
				memmove(&deviationObs, &_deviationObs, 8);
				forecastParameters.prevDeviation.push_back(deviationObs);
			}
			for(size_t n = 0; n < connectionsLen; ++n) {
				forecastParameters.prevConnections.push_back(ntohl(*(uint32_t *) ((connectionsArr + 20) + (n * 8) + 4)));
			}
			forecastParam.insert(make_pair(ip, forecastParameters));
			forecastParameters.clear();
		}
		PQclear(result);
		query.clear();
	}
	//now consolidating data from the day before the closest table as per forecasting offset requirement
	if(!prePrevDate.empty() && offset != 0){
query.assign("SELECT \"internalIP\", \"smoothedObs\", \"trendSmoothedObs\", \"seasonalSmoothedObs\", array_upper(\"smoothedObs\", 1), array_upper(\"trendSmoothedObs\", 1), array_upper(\"seasonalSmoothedObs\", 1), \"deltaSum\" from \"Slowdown\".\"");
		query += prePrevDate + "\"";
		result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL, 1);
		if (PQresultStatus(result) != PGRES_TUPLES_OK) {
			PQclear(result);
			cerr<<PQerrorMessage(postgreSQL)<<endl;
		}
		else{
			uint32_t ip, smoothedObsLen, trendSmoothedObsLen, seasonalSmoothedObsLen;
			uint64_t _smoothedObs;
			uint64_t _trendSmoothedObs;
			uint64_t _seasonalSmoothedObs;
			char *smoothedObsArr;
			char *trendSmoothedObsArr;
			char *seasonalSmoothedObsArr;
			double smoothedObs;
			double trendSmoothedObs;
			double seasonalSmoothedObs;
			vector<double>::iterator posItr;
			size_t pos;
			for (int i = 0; i < PQntuples(result); ++i) {
				ip=ntohl(*(uint32_t*)PQgetvalue(result, i, 0));
				smoothedObsArr = PQgetvalue(result, i, 1);
				trendSmoothedObsArr = PQgetvalue(result, i, 2);
				seasonalSmoothedObsArr = PQgetvalue(result, i, 3);
				smoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 4));
				trendSmoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 5));
				if( trendSmoothedObsLen != BINS_IN_DAY )
					trendSmoothedObsLen =	0;
				seasonalSmoothedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 6));
				forecastParamItr = forecastParam.find(ip);
				if(forecastParamItr != forecastParam.end()){
					pos = 0;
					for(size_t j = offset; j < smoothedObsLen; ++j) {
						_smoothedObs = be64toh(*(uint64_t *)((smoothedObsArr + 20) + (j * 12) + 4));
						memmove(&smoothedObs, &_smoothedObs, 8);
						posItr = forecastParamItr->second.prevSmoothedObs.begin() + pos; ++pos;
						forecastParamItr->second.prevSmoothedObs.insert(posItr, smoothedObs);
					}

					if(trendSmoothedObsLen ==	0){
						forecastParamItr->second.prevTrendSmoothedObs.resize((BINS_IN_DAY / 2));
						generate(forecastParamItr->second.prevTrendSmoothedObs.begin(), forecastParamItr->second.prevTrendSmoothedObs.end(), zero);
					} 
					else{
						pos = 0;
						for(size_t k = offset; k < trendSmoothedObsLen; ++k) {
							_trendSmoothedObs = be64toh(*(uint64_t *)((trendSmoothedObsArr + 20) + (k * 12) + 4));
							memmove(&trendSmoothedObs, &_trendSmoothedObs, 8);
							posItr = forecastParamItr->second.prevTrendSmoothedObs.begin() + pos; ++pos;
							forecastParamItr->second.prevTrendSmoothedObs.insert(posItr, trendSmoothedObs);
							++posItr;
						}
					}

					pos = 0;
					for(size_t l = offset; l < seasonalSmoothedObsLen; ++l) {
						_seasonalSmoothedObs = be64toh(*(uint64_t *)((seasonalSmoothedObsArr + 20) + (l * 12) + 4));
						memmove(&seasonalSmoothedObs, &_seasonalSmoothedObs, 8);
						posItr = forecastParamItr->second.prevSeasonalSmoothedObs.begin();++pos;
						forecastParamItr->second.prevSeasonalSmoothedObs.insert(posItr, seasonalSmoothedObs);
						++posItr;
					}
				}
			}
			PQclear(result);
		}
	} 
}

/* ///\brief function to fetch observed values from previous day
 * ///\param postgreSQL instance of PostgreSQL which specifies DB paramters
 * ///\param date specifies closest previous avaliable table
 */
void fetchPrevObservedData(PGconn *postgreSQL, const string &date) {
	PGresult *result;
string query("SELECT \"internalIP\", \"delta\",	array_upper(\"delta\", 1) from \"Slowdown\".\"");
	query += date + "\"";
	result = PQexecParams(postgreSQL, query.c_str(), 0, NULL, NULL, NULL, NULL, 1);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		PQclear(result);
		cerr<<PQerrorMessage(postgreSQL)<<endl;
	}
	else{
		uint32_t ip, observedObsLen;
		uint64_t _observedObs;
		char *observedObsArr;
		double observedObs;
		vector <double> observed;
		for (int i = 0; i < PQntuples(result); ++i) {
			ip=ntohl(*(uint32_t*)PQgetvalue(result, i, 0));
			observedObsArr = PQgetvalue(result, i, 1);
			observedObsLen =	ntohl(*(uint32_t *) PQgetvalue(result, i, 2));
			for(size_t j = 0; j < observedObsLen; ++j) {
				_observedObs = be64toh(*(uint64_t *)((observedObsArr + 20) + (j * 12) + 4));
				memmove(&observedObs, &_observedObs, 8);
				observed.push_back(observedObs);
			}
			prevObservedObs.insert(make_pair(ip, observed));
			observed.clear();
		}
		PQclear(result);
	}
}


/* ///\brief function to check for host slowdown
 * ///\param readings vector of observed readings
 * ///\param forecasts vector of forecasted readings
 * ///\param firstBinTime first time a value was observed for the ip
 * ///\param alertStartTime is set when slowdown is observed
 * ///\param alertEndTime is set when slowdown is observed
 */
void checkAnomaly(const vector <double> &readings,
				  const vector<double> &confidence,
				  const vector<uint32_t> &connections,
				  const vector<uint32_t> &prevConnections,
				  const uint32_t &firstBinTime,
				  uint32_t &alertStartTime,
				  uint32_t &alertEndTime)
{
	std::vector<double>::const_iterator readingsItr(readings.begin());
	std::vector<double>::const_iterator confidenceItr(confidence.begin());
	std::vector<uint32_t>::const_iterator connectionsItr(connections.begin());
	std::vector<uint32_t>::const_iterator prevConnectionsItr(prevConnections.begin());
	uint32_t anomalyCount = 0; 
	alertStartTime = numeric_limits<uint32_t>::max(); alertEndTime= 0;
	while(confidenceItr != confidence.end() && readingsItr != readings.end()){
		if(*readingsItr > *confidenceItr && *connectionsItr > *prevConnectionsItr){
			++anomalyCount;
		}
		else{
			if(anomalyCount	>= 12){
				compareGreaterThan(alertEndTime,
								   (firstBinTime
								   		+ (distance(readings.begin(),
								   					readingsItr)
												* BIN_SIZE)));
				compareLessThan(alertStartTime,
								(alertEndTime - (anomalyCount * BIN_SIZE)));
			}
			anomalyCount = 0;
		}
		++confidenceItr;
		++readingsItr;
		++connectionsItr;
		++prevConnectionsItr;
	}
	if(!alertEndTime)
		alertStartTime = 0;
	else{
		++slowdownCount;
	}
}

extern "C" {
	bool initialize(SharedState &sharedState, ModuleState &moduleState) {
		connectionInitiators = sharedState.connectionInitiators;
		ipInformation = sharedState.ipInformation;
		localNetworks = sharedState.localNetworks;
		hourStartTime = dayStartTime = getDayStartTime(sharedState.date);
		if (moduleState.conf.get(tcpConnectionTimeout,
								 "tcp-timeout",
								 "analysis_slowdown")
				!= configuration::OK)
		{
			cerr << "analysis_slowdown: missing or invalid tcp-timeout"
				 << endl;
			return false;
		}
		previousTable = getPreviousTable(sharedState.postgreSQL,
										 SLOWDOWN_SCHEMA_NAME,
										 sharedState.date);
		prePreviousTable = getPreviousTable(sharedState.postgreSQL,
											SLOWDOWN_SCHEMA_NAME,
											previousTable.second);
		if(!previousTable.first) {
			cerr<<PQerrorMessage(sharedState.postgreSQL)<<endl;
		}

		if(!previousTable.second.empty()){
			//fetchPrevConsolidatedForecastData(sharedState.postgreSQL,
			//									previousTable.second,
			//									prePreviousTable.second);
			fetchPrevBinData(sharedState.postgreSQL,
							 prevBinTime,
							 previousTable.second);
		}
		else {
			cout<<"First time module is run,no previous Bin values"<<endl;
		}

		return true;
	}

	void aggregate(const FlowStats *flowStats, size_t hour) {
		if(hour != hourTrack) {
			hostStats.clear();
			//cout<<"tcpSmoothing():aggregate(): Spawning smoothCompletedHour() for hour "<<hourTrack<<endl;
			smoothCompletedHour(ipResponseTimes,
								smoothedValues,
								prevBinTime,
								hourTrack,
								hourStartTime,
								smoothedReadings);
			//cout<<"tcpSmoothing():aggregate(): completed smoothCompletedHour() for hour "<<hourTrack<<endl;
			hourStartTime += SECS_IN_HOUR;
			ipResponseTimes.clear();
			++hourTrack;
		}

		if(isTCP(flowStats)){
			/*Get initiator for each flow ID and store SYN and SYN-ACK time*/
			hostPair = makeTwoWayHostPair(flowStats,*localNetworks);
			hostStatsItr = hostStats.find(hostPair);
			if(hostStatsItr == hostStats.end()){
				hostStatsItr = hostStats.insert(
					make_pair(hostPair, HandshakeTimings())).first;
			}

			hostStatsItr->second.initiator = 
				connectionInitiators->getInitiator(hostPair,
												   flowStats->sourceIP());
			if(hostStatsItr->second.initiator == TEMPORARILY_UNKNOWN_INITIATOR){
				connectionInitiators->aggregate(hostPair, flowStats);
				hostStatsItr->second.initiator =
					connectionInitiators->getInitiator(hostPair,
													   flowStats->sourceIP());
			}
				
			if(flowStats->firstSYNTime().seconds()){
				hostStatsItr->second.firstSYNTime = flowStats->firstSYNTime();
			}

			if(flowStats->firstSYNACKTime().seconds()){
				hostStatsItr->second.firstSYNACKTime
					= flowStats->firstSYNACKTime();
			}

			if(flowStats->firstACKTime().seconds()){
				hostStatsItr->second.firstACKTime = flowStats->firstACKTime();
			}

			/*If initiator is internal throw away the data*/
			if(hostStatsItr->second.initiator == PERMANENTLY_UNKNOWN_INITIATOR){
				hostStats.erase(hostPair);
			}
			else{
				//Internal initiator, maintain time series and compute and
				// store time difference between SYN and SYN-ACK
				if(hostStatsItr->second.initiator == INTERNAL_INITIATOR &&
					 isInternal(hostStatsItr->first.internalIP,
					 			*localNetworks) && 
					 hostStatsItr->second.firstSYNACKTime.seconds() && 
					 hostStatsItr->second.firstACKTime.seconds()
					 	>= hourStartTime &&
					 hostStatsItr->second.firstACKTime.seconds()
					 	< (hourStartTime+SECS_IN_HOUR) &&
					 hostStatsItr->second.firstACKTime.seconds() && 
					 (hostStatsItr->second.firstACKTime
					 	> hostStatsItr->second.firstSYNACKTime))
				{
			
					delta = diffTime(hostStatsItr->second.firstSYNACKTime,
									 hostStatsItr->second.firstACKTime);
					if(delta < tcpConnectionTimeout){
						ipResponseTimesItr = ipResponseTimes.find(
							hostStatsItr->first.internalIP);
						if(ipResponseTimesItr == ipResponseTimes.end()){
							ipResponseTimesItr = ipResponseTimes.insert(
								make_pair(hostStatsItr->first.internalIP,
										  ResponseTimings())).first;
						}

						/*storing optimal current measurement*/
						ipResponseTimesItr->second.insert(make_pair(
							hostStatsItr->second.firstACKTime.seconds(),delta));
					}
					 
					/*throw away the flowID data*/
					hostStats.erase(hostPair);
				}
				else{
					if(hostStatsItr->second.initiator == EXTERNAL_INITIATOR &&
						 isInternal(hostStatsItr->first.internalIP,
						 			*localNetworks) &&
						 hostStatsItr->second.firstSYNTime.seconds() &&
						 hostStatsItr->second.firstSYNTime.seconds()
						 	>= hourStartTime &&
						 hostStatsItr->second.firstSYNTime.seconds()
						 	< (hourStartTime+SECS_IN_HOUR) &&
						 hostStatsItr->second.firstSYNACKTime.seconds() &&
						 hostStatsItr->second.firstSYNACKTime
						 	> hostStatsItr->second.firstSYNTime)
					{

						delta = diffTime(hostStatsItr->second.firstSYNTime,
										 hostStatsItr->second.firstSYNACKTime);
						if(delta < tcpConnectionTimeout){
							ipResponseTimesItr = ipResponseTimes.find(
								hostStatsItr->first.internalIP);
							if(ipResponseTimesItr == ipResponseTimes.end()){
								ipResponseTimesItr = ipResponseTimes.insert(
									make_pair(hostStatsItr->first.internalIP,
									ResponseTimings())).first;
							}

							/*storing measurement*/
							ipResponseTimesItr->second.insert(make_pair(hostStatsItr->second.firstSYNTime.seconds(), delta));
						}

						/*throw away the flowID data*/
						hostStats.erase(hostPair);
					}
				}
			}
		}
	}

	int commit(PostgreSQLConnection &pg_conn, size_t &flushSize, const char *date) {
		//cout<<"tcpSmoothing():commit(): Spawning smoothCompletedHour() thread for hour "<<hourTrack<<endl;
		smoothCompletedHour(ipResponseTimes, smoothedValues, prevBinTime,
							hourTrack, hourStartTime, smoothedReadings);
		//cout<<"tcpSmoothing():commit(): completed smoothCompletedHour() for hour "<<hourTrack<<endl;
		ipResponseTimes.clear();
		hostStats.clear();
		prevBinTime.clear();

		Clock clock("Inserted", "rows");
		vector<uint32_t> slowdownIPs;

		//Need a complete days data to run forecast
		if(hourTrack != 23)
			return clock.operations();
			
		PGBulkInserter pgBulkInserter(pg_conn.connection(),
									  SLOWDOWN_SCHEMA_NAME,
									  date,
									  flushSize,
									  "%ud, %ud, %ud, %ud, %ud, %ud, "
									  "%f, %f, %Vf, %Vf, %Vf, %Vf, %Vf, "
									  "%Vf, %Vf, %Vud");
		if (!preparePGTable(pg_conn.connection(),
							SLOWDOWN_SCHEMA_NAME,
							date,
							SLOWDOWN_TABLE_SCHEMA))
		{
			return -1;
		}

		uint32_t lastBinTime=0, alertStartTime=0, alertEndTime=0;
		double lastBinValue = 0, averageReadings=0, sumReadings=0;
		Forecast forecast;

		if(!previousTable.second.empty()){
			fetchPrevConsolidatedForecastData(pg_conn.connection(), previousTable.second, prePreviousTable.second);
		}

		if(!previousTable.second.empty()){
			fetchPrevObservedData(pg_conn.connection(), previousTable.second);
		}

		vector <double> consolidatedDelta;
		vector <double>::iterator conDeltaItr;
		vector <double>::iterator conDeltaBegin;
		vector <double>::iterator conDeltaEnd;

		for(map<uint32_t ,SmoothedReadings>::iterator smoothedValuesItr = smoothedValues.begin();
																									 smoothedValuesItr != smoothedValues.end();
																																				++smoothedValuesItr){
			sumReadings = 0;
			averageReadings=0;
			alertStartTime = 0;
			alertEndTime = 0;
			consolidatedDelta.clear();
			forecast.clear();
			lastBinValue = smoothedValuesItr->second.delta.back();
			lastBinTime = smoothedValuesItr->second.firstBinTime + (smoothedValuesItr->second.delta.size() * BIN_SIZE);
			for(vector<double>::iterator deltaItr = smoothedValuesItr->second.delta.begin();
																	 deltaItr != smoothedValuesItr->second.delta.end();
																	 ++deltaItr){
				sumReadings += *deltaItr;
			}
			averageReadings = sumReadings/smoothedValuesItr->second.delta.size();
			forecastParamItr = forecastParam.find(smoothedValuesItr->first);
			if(forecastParamItr == forecastParam.end()){
				//first day ip is being seen, setting smoothing & seasonal parameters, insufficient data to forecast
				forecast.setParametersNew(averageReadings);
			}
			else{
				if( forecastParamItr->second.prevTrendSmoothedObs.size() == 0){
					//second day ip is being seen, set trend paramter and compute current paramters, insufficient data to forecast
					prevObservedObsItr = prevObservedObs.find(smoothedValuesItr->first);
					if(prevObservedObsItr == prevObservedObs.end()){
						cerr<<"This should never happen, observed values missing for" <<ntop(smoothedValuesItr->first)<<endl;
					}
					if(prevObservedObsItr->second.size() >= FORECAST_AHEAD){
						conDeltaBegin = prevObservedObsItr->second.end() - FORECAST_AHEAD;
						for(conDeltaItr = conDeltaBegin; conDeltaItr != prevObservedObsItr->second.end();
																																							 ++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr);	
						}
						conDeltaEnd = smoothedValuesItr->second.delta.end() - FORECAST_AHEAD;
						for(conDeltaItr = smoothedValuesItr->second.delta.begin(); conDeltaItr != conDeltaEnd;
																																									 ++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr); 
						}
						forecast.setParametersNew(forecastParamItr->second.prevSmoothedObs,
																			forecastParamItr->second.prevSeasonalSmoothedObs,
																			consolidatedDelta,
																			sumReadings,
																			forecastParamItr->second.deltaSum);
						if(consolidatedDelta.size() != BINS_IN_DAY){
							cerr<<"~Consolidation was screwed up" <<ntop(smoothedValuesItr->first)<<endl;
						}
						forecast.computeParameters(consolidatedDelta, FORECAST_AHEAD);
					}
					else{
						//insufficient data after second day, reset count back to first day
						forecast.setParametersNew(averageReadings);
					}
				}
				else{
					//third day ip is being seen, compute paramters, insufficient data to forecast
					if(forecastParamItr->second.prevTrendSmoothedObs.size() == (BINS_IN_DAY/2)){
						forecast.setParameters(forecastParamItr ->second.prevSmoothedObs,
																	 forecastParamItr ->second.prevTrendSmoothedObs,
																	 forecastParamItr ->second.prevSeasonalSmoothedObs,
																	 forecastParamItr ->second.prevDeviation);
						prevObservedObsItr = prevObservedObs.find(smoothedValuesItr->first);
						if(prevObservedObsItr == prevObservedObs.end()){
							cerr<<"This should never happen, observed values missing for "<<ntop(smoothedValuesItr->first)<<endl;
						}
						conDeltaBegin = prevObservedObsItr->second.end() - FORECAST_AHEAD;
						for(conDeltaItr = conDeltaBegin; conDeltaItr != prevObservedObsItr->second.end();
																																							++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr);	
						}
						conDeltaEnd = smoothedValuesItr->second.delta.end() - FORECAST_AHEAD;
						for(conDeltaItr = smoothedValuesItr->second.delta.begin(); conDeltaItr != conDeltaEnd;
																																									 ++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr); 
						}
						if(consolidatedDelta.size() != BINS_IN_DAY){
							cerr<<"~Consolidation was screwed up "<<ntop(smoothedValuesItr->first)<<endl;
						}
						forecast.computeParameters(consolidatedDelta, FORECAST_AHEAD);
					}
					else{
						//sufficient data, forecast and check for slowdown
						forecast.setParameters(forecastParamItr ->second.prevSmoothedObs,
																	 forecastParamItr ->second.prevTrendSmoothedObs,
																	 forecastParamItr ->second.prevSeasonalSmoothedObs,
																	 forecastParamItr ->second.prevDeviation);
						prevObservedObsItr = prevObservedObs.find(smoothedValuesItr->first);
						if(prevObservedObsItr == prevObservedObs.end()){
							cerr<<"This should never happen, observed values missing for" <<ntop(smoothedValuesItr->first)<<endl;
						}
						conDeltaBegin = prevObservedObsItr->second.end() - FORECAST_AHEAD;
						for(conDeltaItr = conDeltaBegin; conDeltaItr != prevObservedObsItr->second.end();
																																							++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr);	
						}
						conDeltaEnd = smoothedValuesItr->second.delta.end() - FORECAST_AHEAD;
						for(conDeltaItr = smoothedValuesItr->second.delta.begin(); conDeltaItr != conDeltaEnd;
																																									 ++conDeltaItr){
							consolidatedDelta.push_back(*conDeltaItr); 
						}
						if(consolidatedDelta.size() != BINS_IN_DAY){
							cerr<<"~Consolidation was screwed up"
								<<ntop(smoothedValuesItr->first)<<endl;
						}
						if(forecast.holtWinters(FORECAST_AHEAD,
												consolidatedDelta))
						{
							checkAnomaly(smoothedValuesItr->second.delta,
										 forecast.confidence,
										 smoothedValuesItr->second.connections,
										 forecastParamItr
										 	->second.prevConnections,
										smoothedValuesItr->second.firstBinTime,
										alertStartTime,
										alertEndTime);
						}
						else{
							//cout<<"Forecasting went out of bounds for "<<ntop(smoothedValuesItr->first)<<". Restarting time series"<<endl;
							continue;
						}
					}
				}
			}

						/*cout<<ntop(smoothedValuesItr->first)<<"\t"
								<<smoothedValuesItr->second.firstBinTime<<"\t"
								<<lastBinTime<<"\t"
								<<dayStartTime<<"\t"
								<<alertStartTime<<"\t"
								<<alertEndTime<<"\t"
								<<lastBinValue<<"\t"
								<<sumReadings<<"\t"
								<<forecastParamItr->second.deltaSum<<"\t"
								<<forecast.StL.size()<<"\t"
								<<forecast.BtL.size()<<"\t"
								<<forecast.ItL.size()<<"\t"
								<<forecastParamItr->second.prevSmoothedObs.size()<<"\t"
								<<forecastParamItr->second.prevTrendSmoothedObs.size()t"
								<<forecastParamItr->second.prevSeasonalSmoothedObs.size()<<"\t"
								<<endl;*/
			if(alertStartTime) {
				slowdownIPs.push_back(smoothedValuesItr->first);
			}
			if(!pgBulkInserter.insert(NULL,
								smoothedValuesItr->first,
								smoothedValuesItr->second.firstBinTime,
								lastBinTime,
								dayStartTime,
								alertStartTime,
								alertEndTime,
								lastBinValue,
								sumReadings,
								(void*)&forecast.StV,
								(void*)&forecast.BtV,
								(void*)&forecast.ItV,
								(void*)&smoothedValuesItr->second.delta,
								(void*)&forecast.forecasts,
								(void*)&forecast.DtV,
								(void*)&forecast.confidence,
								(void*)&smoothedValuesItr->second.connections))
			{
				return -1;
			}
			clock.incrementOperations();
		}


		if (pgBulkInserter.size() && !pgBulkInserter.flush()) {
			return -1;
		}
		cout<<"Slowdown: "<<slowdownCount<<endl;
		/*for(vector<uint32_t>::iterator itr = slowdownIPs.begin();
																	 itr != slowdownIPs.end();
																									++itr){
			cout<<ntop(*itr)<<endl;
		}*/
		return clock.operations();
	}
}
