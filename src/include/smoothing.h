///
///\file 	smoothing.h
///		This header does smoothing to remove noise from time series and makes it discrete
///

#ifndef SMOOTHING_H
#define SMOOTHING_H

#include <map>
#include <vector>
#include <set>
#include <math.h>

#define BIN_SIZE      300
#define BINS_IN_HOUR  12
#define BINS_IN_DAY   288
#define SECS_IN_HOUR  3600
#define MEDIAN_FILTER_MAX_SIZE  5

///class to store value type for disk based Data strucure
class SmoothedReadings{
public:
        uint32_t firstBinTime; //<stores the time first bin was populated
        std::vector <double> delta; //<series of discrete smooothed observations
        std::vector <uint32_t> connections;
        SmoothedReadings();
};

///\brief Overloaded () operator for custom strict weak ordering key in set
struct ltKeyPair
{
  bool operator()(const std::pair <uint32_t, double> p1, const std::pair <uint32_t, double> p2) const
  {
    if(p1.first != p2.first){
      return(p1 < p2);
    }
    else{
      return false;
    }
  }
};

///\brief type definition for container to store observed timings
typedef std::set <std::pair <uint32_t, double>, ltKeyPair > ResponseTimings;
typedef std::map <uint32_t, ResponseTimings> IPResponseTimes;

///\brief type definition for container to store smoothed bins
typedef std::map <uint32_t, SmoothedReadings> SmoothedValues;

///\brief type definition for container to store the last seen bin time for each IP to enable visibility across hours
typedef std::map <uint32_t, std::pair<uint32_t, double> > PrevBinTime;


///\Function to store last bin time for current processed hour
///\param ip specifies ip to check for existing time series
///\param prevBinTime map which contains mapping from ip to last bin time
///\return true if ip has existing time series and false if not
bool isSeenIP(const uint32_t &ip, PrevBinTime &prevBinTime);


///\Function to store last bin time for current processed hour
///\param ip specifies ip for which last bin time need to be updated
///\param binTime specifies updated last bin time
///\param prevBinTime map which contains mapping from ip to last bin time and bin value
void storePrevBinTime(const uint32_t &ip, const uint32_t &binTime, 
                              const double &binValue, PrevBinTime &prevBinTime);

///\Function to retreive last bin time for given ip
///\param ip specifies ip for which last bin time need to be retreived
///\param prevBinTime map which contains mapping from ip to last bin time
///\return last bin start time for the requested ip
uint32_t getLastBinEndTime(const uint32_t &ip, PrevBinTime &prevBinTime);


///\Function to retreive last bin value for given ip
///\param ip specifies ip for which last bin time need to be retreived
///\param prevBinTime map which contains mapping from ip to last bin time and bin value
///\return value of last bins given ip
double getLastBinValue(const uint32_t &ip, PrevBinTime &prevBinTime);


///\brief function to fill bins in memory one ip at a time
///\param smoothedValuesItr is and iterator pointing to current IP being sorted in to bins
///\param ip specifies IP for which bins need to be comitted to disk
///\param smoothedReadings object is stored as value in map at the end of the hour
///\param binIndex maintains index in array for current ip
///\param value specifies value to be filled in array
///\param firstBinTime stores the first bin time in current hour for ip
///\param count specifies number of values to fill in array
///\param count specifies number connection seen in time period
void fillBins( std::map <uint32_t, SmoothedReadings>::iterator smoothedValuesItr, const uint32_t &ip,
                                SmoothedReadings &smoothedReadings, uint32_t &binIndex, double &value,
                   const uint32_t &firstBinTime, const uint32_t &count, const uint32_t &connectionCount);


///\brief function to apply median filtering to noisy data
///\param delta is a set containing delay measurements in pair.second and ordered by time order in pair.first
///\param neighborhood specifies number of spatial moments to use for median filtering
///\return a vector of filtered observations  
std::vector<std::pair <uint32_t, double> > medianFilter(const ResponseTimings &delta, const uint32_t &neighborhood) ;

///\brief function at the end of each hour to smooth completed hour. Each time a new value is seen in the time  
/// series checks are done to see if time bin has elapsed and if it has elapsed, time series is smoothed 
/// in to bins of specified interval which is done by determining the current interval offset from start time of  
/// previous time bin.
///\param ipResponseTimes stores current hours response times 
///\param smoothedValues is the map containing smoothed bins for each IP
///\param prevBinTime map which contains mapping from ip to last bin time and bin value
///\param hour current hour being smoothed
///\param smoothedReadings object is stored as value in map at the end of the hour
void smoothCompletedHour(IPResponseTimes &ipResponseTimes, SmoothedValues &smoothedValues, PrevBinTime &prevBinTime, 
                              const size_t &hour, const uint32_t &hourStartTime, SmoothedReadings &smoothedReadings);

#endif

