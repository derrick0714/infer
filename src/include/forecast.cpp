#include "forecast.h"
#include <iostream>
#include <algorithm>
#include <math.h>

ForecastParameters::ForecastParameters(){
        deltaSum=0;
        prevSmoothedObs.clear();
        prevTrendSmoothedObs.clear();
        prevSeasonalSmoothedObs.clear();
        prevDeviation.clear();
        prevConnections.clear();
}

void ForecastParameters::clear(){
        deltaSum=0;
        prevSmoothedObs.clear();
        prevTrendSmoothedObs.clear();
        prevSeasonalSmoothedObs.clear();
        prevDeviation.clear();
        prevConnections.clear();
}	

Forecast::Forecast(){
        //the season defaults to one day
        L=(24 * 60 * 60) / PERIOD_SIZE;
        //Formula for parameters : alpha/beta/gamma = 1 - exp((ln(1 - % weight) / # of time points))
        //reasonable values for alpha are between 0 and 1 
        alpha=0.1745; //observations in last 1 hour accounts for 90% of weights
        beta=0.0024; //observations in last 1 day account for 50% of weights
        gamma=0.0331; //observations in last 7 days account for 75% weights
        //reasonable values for sigma are between 2 and 3
        sigma=3;  
        St=0;
        Bt=0;
        It=0;
        Ft=0;
        forecasts.clear();
        StV.clear();
        BtV.clear();
        ItV.clear();
        DtV.clear();
        StL.clear();
        BtL.clear();
        ItL.clear();
        DtL.clear();
}

void Forecast::clear(){
        St=0;
        Bt=0;
        It=0;
        Ft=0;
        forecasts.clear();
        confidence.clear();
        StV.clear();
        BtV.clear();
        ItV.clear();
        DtV.clear();
        StL.clear();
        BtL.clear();
        ItL.clear();
        DtL.clear();
}

void Forecast::setAlphaBetaGammaSigma(const double &_alpha, const double &_beta, 
                                      const double &_gamma, const double &_sigma){
	this->alpha = _alpha;
	this->beta = _beta;
	this->gamma = _gamma;
	this->sigma = _sigma;
}

void Forecast::setSeason(const double &_season){
	this->L = _season;
}

void Forecast::computeParameters(const std::vector <double> &readings, const uint32_t  &forecastAhead){
        std::vector <double>:: iterator ItLItr(ItL.begin());
        std::vector <double>:: iterator DtLItr(DtL.begin());
        for(std::vector<double>::const_iterator readingsItr = readings.begin();
                                     readingsItr != readings.end();
                                     ++readingsItr){

          //Multiplicative model
          /*if( StV.empty()){
            St = alpha * ( *(readingsItr) / *(ItLItr) ) + (1 - alpha) * (StL.back() + BtL.back());
            Bt = beta * ( St - StL.back()) + (1 - beta) * BtL.back());
            It = gamma * ( *(readingsItr) / St) + (1 - gamma) * (*(ItLItr));
          }
          else{
            St = alpha * ( *(readingsItr) / *(ItLItr) ) + (1 - alpha) * (StV.back() + BtV.back());
            Bt = beta * ( St - StV.back()) + (1 - beta) * BtV.back());
            It = gamma * ( *(readingsItr) / St) + (1 - gamma) * (*(ItLItr));
          }
          seasonOffset = ItLItr + forecastAhead;
          Ft = (St + forecastAhead * Bt) * *(seasonOffset);*/

          //Additive Model accounts for varied seasonality
          if( StV.empty()){
            St = alpha * ( *(readingsItr) - *(ItLItr) ) + (1 - alpha) * (StL.back() + BtL.back());
            Bt = beta * ( St - StL.back()) + (1 - beta) * BtL.back();
            It = gamma * ( *(readingsItr) - St) + (1 - gamma) * (*(ItLItr));
          }
          else{
            St = alpha * ( *(readingsItr) - *(ItLItr) ) + (1 - alpha) * (StV.back() + BtV.back());
            Bt = beta * ( St - StV.back()) + (1 - beta) * BtV.back();
            It = gamma * ( *(readingsItr) - St) + (1 - gamma) * (*(ItLItr));
          }
          Ft = (St + forecastAhead * Bt) + *(ItLItr);
          Dt = gamma * abs(*(readingsItr) - Ft) + ((1 - gamma) * (*(DtLItr)));

          StV.push_back(St); BtV.push_back(Bt); ItV.push_back(It);
          DtV.push_back(Dt);
          ++ItLItr;
          ++DtLItr;
        }
}

bool Forecast::holtWinters(const uint32_t &forecastAhead, const std::vector <double> &readings){
        std::vector <double>:: iterator ItLItr(ItL.begin());
        std::vector <double>:: iterator DtLItr(DtL.begin());
        for(std::vector<double>::const_iterator readingsItr = readings.begin();
                                     readingsItr != readings.end();
                                     ++readingsItr){
          //sanity checks
          if(!StL.size() || !BtL.size() || !ItL.size()){
            std::cerr<<"holtWinters()::Forecast paramters not set. Did you forget to call setParamters() ?"<<std::endl;
            return false;
          }
          if(*ItLItr == 0 || isnan(this->St) || isnan(this->Bt) || isnan(this->It) ||
                              isinf(this->Bt)|| isinf(this->St) || isinf(this->It) ){
            return false;
          }

          //Multiplicative model
          /*if( StV.empty()){
            St = alpha * ( *(readingsItr) / *(ItLItr) ) + (1 - alpha) * (StL.back() + BtL.back());
            Bt = beta * ( St - StL.back()) + (1 - beta) * BtL.back());
            It = gamma * ( *(readingsItr) / St) + (1 - gamma) * (*(ItLItr));
          }
          else{
            St = alpha * ( *(readingsItr) / *(ItLItr) ) + (1 - alpha) * (StV.back() + BtV.back());
            Bt = beta * ( St - StV.back()) + (1 - beta) * BtV.back());
            It = gamma * ( *(readingsItr) / St) + (1 - gamma) * (*(ItLItr));
          }
          seasonOffset = ItLItr + forecastAhead;
          Ft = (St + forecastAhead * Bt) * *(seasonOffset);*/

          //Additive Model accounts for varied seasonality
          if( StV.empty()){
            St = alpha * ( *(readingsItr) - *(ItLItr) ) + (1 - alpha) * (StL.back() + BtL.back());
            Bt = beta * ( St - StL.back()) + (1 - beta) * BtL.back();
            It = gamma * ( *(readingsItr) - St) + (1 - gamma) * (*(ItLItr));
          }
          else{
            St = alpha * ( *(readingsItr) - *(ItLItr) ) + (1 - alpha) * (StV.back() + BtV.back());
            Bt = beta * ( St - StV.back()) + (1 - beta) * BtV.back();
            It = gamma * ( *(readingsItr) - St) + (1 - gamma) * (*(ItLItr));
          }
          Ft = (St + forecastAhead * Bt) + *(ItLItr);
          Dt = gamma * abs(*(readingsItr) - Ft) + ((1 - gamma) * (*(DtLItr)));
          Ct = Ft + (sigma * (*(DtLItr)));

          //forecast and confidence plot is bound to 0+ since network data doesnt exhibit negative data
          //if(Ft < 0)
            //Ft = 0;
          //if(Ct < 0)
            //Ct = 0;

          StV.push_back(St); BtV.push_back(Bt); ItV.push_back(It); forecasts.push_back(Ft);
          DtV.push_back(Dt); confidence.push_back(Ct);
          ++ItLItr;
          ++DtLItr;
        }
        return true;
}

void Forecast::setParameters(const std::vector<double> &_StL, const std::vector <double> &_BtL, 
                             const std::vector <double> &_ItL, const std::vector <double> &_DtL){
        this->StL = _StL;
        this->BtL = _BtL;
        this->ItL = _ItL;
        this->DtL = _DtL;
}

void Forecast::setParametersNew(const std::vector <double> &_StL, const std::vector <double> &_ItL, 
                                  const std::vector <double> &_readings, const double &sumReadings, 
                                                                 const double &previousSumReadings){
        StL = _StL;
        ItL = _ItL;
        double trend = (sumReadings - previousSumReadings) / (L*L);
        double previousAverage = previousSumReadings / BINS_IN_DAY; 
        for(size_t i = 0; i < BINS_IN_DAY; ++i){
            BtL.push_back(trend);
            DtL.push_back(abs(_readings.at(i) - previousAverage));
        }
}

void Forecast::setParametersNew(const double &averageReadings){
        double season = averageReadings / L;
        for(size_t i = 0; i < BINS_IN_DAY; ++i){
            ItV.push_back(season);
            StV.push_back(0);
        }
}

