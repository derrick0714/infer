///
///\file	forecast.h
///\		This header provides a class that helps in forecasting future time series using holt winters

#ifndef FORECAST_H
#define FORECAST_H
#include <vector>

#define PERIOD_SIZE 300
#define BINS_IN_DAY 288

///container used to keep forecasting paramaters for each season in modules using forecasting
class ForecastParameters{
public:
        double deltaSum;
        std::vector <double> prevSmoothedObs;
        std::vector <double> prevTrendSmoothedObs;
        std::vector <double> prevSeasonalSmoothedObs;
        std::vector <double> prevDeviation; 
        std::vector <uint32_t> prevConnections; 
        ForecastParameters();
        void clear();
};

///class which implements holt winters forecasting algorithm
class Forecast{
public:

        //Paramters used in forecasting
        double L,     //<Number of periods that complete 1 season(season is 1 day)
               St,    //<Smoothed observation at time t
               Bt,    //<Trend Smoothing at time t
               It,    //<Seasonal smoothing at time t
               Ft,    //<Forecast prediction one bin ahead
               Dt,    //deviation prediction one period ahead
               Ct,    //confidence upper bound one period ahead
               alpha, //<Overall smoothing parameter
               beta,  //<Seasonal smoothing paramter
               gamma, //<Trend smoothing paramter
               sigma; //<Confidence band scaling factor
        std::vector <double> forecasts, //<used to store computed forecasts
                             confidence, //<used to store computed confidence upper bound
                             StL,   //<Smoothed observations from previous period
                             BtL,   //<Trend Smoothing from previous period
                             ItL,   //<Seasonal smoothing from previous period
                             DtL,   //deviation prediction from previous period
                             StV,    //<Smoothed observations from current period
                             BtV,    //<Trend Smoothing observations from current period
                             ItV,    //<Seasonal smoothing observations from current period
                             DtV;    //<deviation prediction from current period


        ///\brief Default contructor for Forecast() class which initializes alpha,beta and gamma and season
        ///       Here seasons period is one day and number of interval in season is (24*60)/BIN_SIZE
        ///       *Alpha is basically the short term parameter. A large value will give a large
        ///        weight to measurements very near in the past, while a small value of alpha will
        ///        give more weight to measurements further in the past.Determines how responsive a forecast 
        ///        is to sudden jumps and drops. It is the percentage weight given to the prior period, 
        ///        and the remainder is distributed to the other historical periods. The lower the value of alpha, 
        ///        the less responsive the forecast is to sudden change. 
        ///       *Gamma is the seasonal parameter. A big value will give a big weight to the
        ///        present relation between the observation and the smoothed observation, and little
        ///        values will give more weight to past relation between the observation and the smoothed observation. 
        ///        Determines how sensitive a forecast is to seasonal factors. The smaller the value of gamma, the less 
        ///        weight is given to seasonal factors.
        ///       *Beta is the trend parameter. A big value will give more weight to the
        ///        difference of the last smoothed observations; while a little value will use
        ///        information further in the past. Determines how sensitive a forecast is to the trend. The smaller 
        ///        the value of beta, the less weight is given to the trend. The value of beta is usually small, because 
        ///        trend is a long-term effect.
        Forecast();

        ///\brief Member function that clear smoothing paramaters and forecasts
        void clear();

        ///\brief Member function to set alpha, beta and gamma values to other than default
        void setAlphaBetaGammaSigma(const double &alpha, const double &beta, 
                                    const double &gamma, const double &sigma);
 
        ///\brief Member function to set season to other than default
        ///\param season to is the value to be set
        void setSeason(const double &_season);

        ///\brief function used to compute initial forecasting paramters
        ///\param readings is a vector of observed readings in time series
        ///\param forecastAhead specifies number of periods ahead to forecast
        void computeParameters(const std::vector <double> &readings, const uint32_t &forecastAhead);

        ///\brief Implementation of Holf winters algorithm to predict future response times of a host
        ///       under the assumption that the future will follow the same behaviour as the past
        ///       Since our seasonal factor here is clearly daily periodicity we model our season
        ///       paramter around a days data.
        ///\param forecastAhead specifies number of periods ahead to forecast
        ///\param readings is a vector of observed readings in time series
        ///\return boolean value true on succesful forecast
        bool holtWinters(const uint32_t &forecastAhead, const std::vector <double> &readings);

        ///\brief function that sets paramters for forecasting from previous days data
        ///\param _StL is the previous periods smoothed values
        ///\param _BtL is the previous periods trend smoothed values 
        ///\param _ItL is the previous periods seasonal smoothed values 
        ///\param _DtL is the previous periods deviation values
        void setParameters(const std::vector<double> &_StL, const std::vector <double> &_BtL,
                           const std::vector <double> &_ItL, const std::vector <double> &_DtL);

        ///\brief function that sets paramters for forecasting for time series with only 1 days data
        ///             trend paramters are set.
        ///\param ItL here is vector for seasonal smoothed readings from previous period
        ///\param readings here is vector of current readings
        ///\param sumReadings is the sum of observed readings 
        ///\param previousSumReadings is the sum of previous days observed readings 
        void setParametersNew(const std::vector <double> &_StL, const std::vector <double> &_ItL, 
                                const std::vector <double> &_readings, const double &sumReadings, 
                                                               const double &previousSumReadings);

        ///\brief function that sets paramters for forecasting for a new time series
        ///             seasonal parameters and smoothing parameter set at this point
        ///             smoothing paramters are initialized to 0 for new time series
        ///\param averageReadings here is simply the aveage of observed readings
        ///\param averageHourReadings here is the observed average value for each hour of the day
        void setParametersNew(const double &averageReadings);
};

#endif
