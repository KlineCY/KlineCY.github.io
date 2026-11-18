#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double american_crr_option(double spot,double strike,double rate,
         double dividend_yield,double volatility,double maturity,
         int steps,bool call){
    require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(rate)&&std::isfinite(dividend_yield)&&
            std::isfinite(volatility)&&volatility>0&&
            std::isfinite(maturity)&&maturity>0&&steps>=1&&steps<=20000,
            "Invalid CRR option inputs");
    double dt=maturity/steps,up=std::exp(volatility*std::sqrt(dt)),
           down=1/up,p=(std::exp((rate-dividend_yield)*dt)-down)/
                         (up-down);
    require(p>=0&&p<=1,"CRR steps violate risk neutral probability bounds");
    double discount=std::exp(-rate*dt);
    std::vector<double> values(steps+1);
    for(int j=0;j<=steps;j++){
        double price=spot*std::pow(up,j)*std::pow(down,steps-j);
        values[j]=std::max(0.0,call?price-strike:strike-price);
    }
    for(int t=steps-1;t>=0;t--){
        for(int j=0;j<=t;j++){
            double price=spot*std::pow(up,j)*std::pow(down,t-j);
            double exercise=std::max(0.0,call?price-strike:strike-price);
            double hold=discount*(p*values[j+1]+(1-p)*values[j]);
            values[j]=std::max(exercise,hold);
        }
    }
    return values[0];
}
} // namespace kline
