
#include "smoothing.h"
#include <iostream>
#include <assert.h>

SmoothedReadings::SmoothedReadings(){
        firstBinTime = 0;
        delta.clear();
	connections.clear();
}

bool isSeenIP(const uint32_t &ip, PrevBinTime &prevBinTime){
        std::map <uint32_t, std::pair<uint32_t, double> >::const_iterator prevBinTimeItr;
        prevBinTimeItr = prevBinTime.find(ip);
        if(prevBinTimeItr == prevBinTime.end()){
          return false;
        }
        else{
          return true;
        }
}

void storePrevBinTime(const uint32_t &ip, const uint32_t &binTime,
                              const double &binValue, PrevBinTime &prevBinTime){
        std::map <uint32_t, std::pair<uint32_t,double> >::iterator prevBinTimeItr;
        prevBinTimeItr = prevBinTime.find(ip);
        if(prevBinTimeItr == prevBinTime.end()){
          prevBinTimeItr = prevBinTime.insert(make_pair(ip, std::pair<uint32_t, double>())).first;
        }
        prevBinTimeItr->second.first = binTime;
        prevBinTimeItr->second.second = binValue;
}

uint32_t getLastBinEndTime(const uint32_t &ip, PrevBinTime &prevBinTime) {
        std::map <uint32_t, std::pair<uint32_t,double> >::const_iterator prevBinTimeItr;
        prevBinTimeItr = prevBinTime.find(ip);
        if(prevBinTimeItr == prevBinTime.end()){
            std::cerr<<"tcpSmoothing():getLastBinEndTime(): Error retreiving last bin time"<<std::endl;
        }
        return prevBinTimeItr->second.first;
}

double getLastBinValue(const uint32_t &ip, PrevBinTime &prevBinTime) {
        std::map <uint32_t, std::pair<uint32_t,double> >::const_iterator prevBinTimeItr;
        prevBinTimeItr = prevBinTime.find(ip);
        if(prevBinTimeItr == prevBinTime.end()){
            std::cerr<<"tcpSmoothing():getLastBinValue(): Error retreiving last bin value"<<std::endl;
        }
        return prevBinTimeItr->second.second;
}

void fillBins( std::map <uint32_t, SmoothedReadings>::iterator smoothedValuesItr, const uint32_t &, 
                                SmoothedReadings &, uint32_t &binIndex, double &value, 
                   const uint32_t &firstBinTime, const uint32_t &count, const uint32_t &connectionCount){
        if(!smoothedValuesItr->second.firstBinTime)
          smoothedValuesItr->second.firstBinTime  = firstBinTime;
        double _value = value;
        if(count > 1){
          double bins;
	  for(size_t index = 0; index < count - 1; ++index){
            bins = 0;
            value = 0;
            /*copying average of last 24 bins when bins are copied(subject to availability)*/
            for(std::vector<double>::reverse_iterator rit =  smoothedValuesItr->second.delta.rbegin();
                                                        rit != smoothedValuesItr->second.delta.rend();
                                                        ++rit){
              value += *rit; 
              ++bins;
              if(bins == 24)
                break;
            }
            if(bins)
              value /= bins;
            if(!value && smoothedValuesItr->second.delta.empty())
              value = _value;
            smoothedValuesItr->second.delta.push_back(value);
            smoothedValuesItr->second.connections.push_back(0);
            ++binIndex;
          }
        }
        smoothedValuesItr->second.delta.push_back(_value);
        smoothedValuesItr->second.connections.push_back(connectionCount);
        ++binIndex;
        /*Ensures bound check for 12 bins in an hour, can be turned off later by adding #define NDEBUG*/
        assert(binIndex <= BINS_IN_HOUR);

}

std::vector<std::pair <uint32_t, double> > medianFilter(const ResponseTimings &delta, const uint32_t &neighborhood){
        std::vector<double> filter;
        std::pair<uint32_t, double> copy;
        std::vector<std::pair <uint32_t, double> > filtered;
        std::set <std::pair <uint32_t, double>, ltKeyPair >::iterator rTimingsBegin;
        std::set <std::pair <uint32_t, double>, ltKeyPair >::iterator rTimingsEnd;
        std::set <std::pair <uint32_t, double>, ltKeyPair >::iterator filterFiller;

        for(std::set <std::pair <uint32_t, double>, ltKeyPair >::const_iterator rTimingsItr = delta.begin();
                                                                           rTimingsItr != delta.end();
                                                                                      ++rTimingsItr) {
          rTimingsEnd = delta.end();
          rTimingsBegin = delta.begin();
          filterFiller = rTimingsItr;
          filter.clear();

          //obtaining neighborhood moments and populating filter vector
          while(filterFiller != rTimingsEnd && filter.size() != neighborhood){
            filter.push_back(filterFiller->second);
            ++filterFiller;
          }
          if(filter.size() != neighborhood && rTimingsItr != rTimingsBegin){
            filterFiller = rTimingsItr;
            do{
              --filterFiller;
              filter.push_back(filterFiller->second);
            }while(filterFiller != rTimingsBegin && filter.size() != neighborhood);
          }          

          //Constructing filtered data
          sort(filter.begin(), filter.end());
          copy.first = rTimingsItr->first;
          copy.second = *(filter.begin() + (filter.size() / 2));
          filtered.push_back(copy);
        } 
        return filtered;
}


void smoothCompletedHour(IPResponseTimes &ipResponseTimes, SmoothedValues &smoothedValues, 
                                       PrevBinTime &prevBinTime, const size_t &, const uint32_t &hourStartTime, 
                                                      SmoothedReadings &smoothedReadings){

            /*Variables that maintain bin states for each IP*/
            uint32_t currentInterval, binReadingCount, binStartTime, currentTime, binIndex, 
                                                                firstBinTime=0;
            std::vector <double> bins;
            double minBinReading=0, previousBinReading=0;
            std::set <uint32_t> processedIPs;
            std::vector<std::pair <uint32_t, double> > filteredDelta;
            std::vector<std::pair <uint32_t, double> >::iterator rTimingsLastElement;
            std::map<uint32_t, SmoothedReadings>::iterator smoothedValuesItr(smoothedValues.end());

	    /*Looping through completed hour's data*/
            for(std::map <uint32_t, ResponseTimings>::const_iterator ipRTimesItr = ipResponseTimes.begin();
                                                          ipRTimesItr != ipResponseTimes.end();
                                                          ++ipRTimesItr) {
              smoothedValuesItr = smoothedValues.end();
              binIndex=0;firstBinTime=0;binReadingCount=0;
              processedIPs.insert(ipRTimesItr->first);
              filteredDelta.clear();

              /*applying median filter with 5 spatial moments*/
              filteredDelta = medianFilter(ipRTimesItr->second, MEDIAN_FILTER_MAX_SIZE);

              /*If IP is new add it to smoothed timings and set bin start time as current hours start time*/
              if(!isSeenIP(ipRTimesItr->first, prevBinTime)) {
	        smoothedValuesItr = smoothedValues.insert(std::make_pair(ipRTimesItr->first, SmoothedReadings())).first;
                binStartTime = hourStartTime;
                minBinReading = previousBinReading = filteredDelta.begin()->second;
                firstBinTime = binStartTime;
              }
              /*If IP already exists in smoothed timings map read out the last bin time from prevBinTime map,
                move the bin offset to current hour*/
              else{
                smoothedValuesItr = smoothedValues.find(ipRTimesItr->first);
                if(smoothedValuesItr == smoothedValues.end())
	          smoothedValuesItr = smoothedValues.insert(std::make_pair(ipRTimesItr->first, SmoothedReadings())).first;
                binStartTime = getLastBinEndTime(ipRTimesItr->first, prevBinTime);
                if(binStartTime > hourStartTime)
                  binStartTime = hourStartTime;
                while(binStartTime < hourStartTime){
                  binStartTime += BIN_SIZE;
                }
                previousBinReading = getLastBinValue(ipRTimesItr->first, prevBinTime);
                minBinReading = filteredDelta.begin()->second;
                if(!firstBinTime){
                  firstBinTime = binStartTime;
                }
              }


              rTimingsLastElement = filteredDelta.end(); --rTimingsLastElement;
              for(std::vector <std::pair <uint32_t, double> >::const_iterator rTimingsItr = filteredDelta.begin();
                                                                               rTimingsItr != filteredDelta.end();
                                                                                                  ++rTimingsItr) {
                currentTime = rTimingsItr->first;
                
	        /*This bucket was already closed last hour, pushing value to new bucket*/
                if(binStartTime > currentTime ){
                  currentTime = binStartTime;
                }
           
                /*Detemining bin interval*/
                currentInterval = (currentTime - binStartTime)/BIN_SIZE;

                /*Either a value in a new bin was seen OR we know no further readings exist for this IP*/
                if(currentInterval > 0 || rTimingsItr == rTimingsLastElement){
                  if(rTimingsItr == rTimingsLastElement){
                    /*Still in current interval, just close open bin*/
                    if(currentInterval == 0){
                      bins.push_back(rTimingsItr->second);
                      minBinReading = *(bins.begin() + bins.size()/2) ;
                      ++binReadingCount;
                      fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, minBinReading, firstBinTime, 1, binReadingCount);
                      binStartTime += BIN_SIZE;
                    }
                    /*Fill all missed intervals and current interval*/
                    else{
                      if(binReadingCount){
                        fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, minBinReading, firstBinTime, 
                                                                                                     currentInterval, binReadingCount);
                      }
                      else{
                        fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, previousBinReading, firstBinTime, 
                                                                                                     currentInterval, binReadingCount);
                      }
                      /*Also write current new bin since no more values will be seen*/
                      minBinReading = rTimingsItr->second;
                      /*Additional +1 since we also close current bin*/
                      binStartTime += (currentInterval+1) * BIN_SIZE;
                      fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, minBinReading, firstBinTime, 1, 1);
                      previousBinReading = minBinReading;
                    }
                    /*Now lets make sure we pad empty bins for this hour*/
                    if(binIndex < BINS_IN_HOUR){
                      fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, previousBinReading, firstBinTime, 
                                                                                                           BINS_IN_HOUR-binIndex, 0);
                      binStartTime += (BINS_IN_HOUR-binIndex-1) * BIN_SIZE; 
                    }
                    bins.clear();
                  }
                  /*Close open bin, fill missing intervals and open new bin*/
                  else{
                    if(binReadingCount){
                      fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, minBinReading, firstBinTime, currentInterval, 
                                                                                                                                  binReadingCount);
                      binReadingCount = 1;
                      previousBinReading = minBinReading;
                      minBinReading = rTimingsItr->second;
                      binStartTime += currentInterval * BIN_SIZE;
                    }
                    else{
                      fillBins(smoothedValuesItr, ipRTimesItr->first, smoothedReadings, binIndex, previousBinReading, firstBinTime, 
                                                                                                                 currentInterval, binReadingCount);
                      binReadingCount = 1;
                      minBinReading = rTimingsItr->second;
                      binStartTime += currentInterval * BIN_SIZE;
                    }
                    bins.clear();
                  } 
                }
                /*Current bin value was read, Just updating minimum bin reading and count */
                else{
                  bins.push_back(rTimingsItr->second);
                  minBinReading = *(bins.begin() + bins.size()/2) ;
                  ++binReadingCount;
                }
              }
              /*Done with current IP, store last bin time and last bin value*/
              storePrevBinTime(ipRTimesItr->first, binStartTime, previousBinReading, prevBinTime);
              filteredDelta.clear();
            }
            ipResponseTimes.clear();

            /*Padding bins for IP's seen in previous hours but not in the current hour*/
            for(std::map <uint32_t, std::pair<uint32_t, double> >::iterator prevBinTimeItr = prevBinTime.begin();
                                                                       prevBinTimeItr != prevBinTime.end();
                                                                       ++prevBinTimeItr) {
              smoothedValuesItr = smoothedValues.find(prevBinTimeItr->first);
              if(smoothedValuesItr == smoothedValues.end())
	        smoothedValuesItr = smoothedValues.insert(std::make_pair(prevBinTimeItr->first, SmoothedReadings())).first;
              if(processedIPs.find(prevBinTimeItr->first) == processedIPs.end()){
                binIndex=0;
                firstBinTime = prevBinTimeItr->second.first;
                if(firstBinTime > hourStartTime){
                  firstBinTime = hourStartTime;
                }
                while(firstBinTime < hourStartTime){
                  firstBinTime += BIN_SIZE;
                }
                fillBins(smoothedValuesItr, prevBinTimeItr->first, smoothedReadings, binIndex, prevBinTimeItr->second.second, firstBinTime, 
                                                                                                                           BINS_IN_HOUR, 0);
                prevBinTimeItr->second.first += BIN_SIZE * BINS_IN_HOUR;
              }
            }
            processedIPs.clear();
}	
