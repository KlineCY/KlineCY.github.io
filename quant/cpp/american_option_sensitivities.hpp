#pragma once
#include "derivatives.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct BinomialGreeks{double value=0,delta=0,gamma=0,vega=0;};
inline BinomialGreeks american_binomial_greeks(double spot,double strike,
    double rate,double dividend,double volatility,double years,
    int steps,bool call=true){
    require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(volatility)&&volatility>0&&years>0&&
            steps>=3,"Invalid American Greek contract");
    double ds=std::max(1e-3,spot*1e-3);
    double dv=std::max(1e-4,volatility*1e-3);
    auto price=[&](double s,double v){
        return american_binomial(s,strike,rate,dividend,v,years,steps,call);
    };
    double center=price(spot,volatility);
    double plus=price(spot+ds,volatility);
    double minus=price(spot-ds,volatility);
    return {center,(plus-minus)/(2*ds),
            (plus-2*center+minus)/(ds*ds),
            (price(spot,volatility+dv)-price(spot,volatility-dv))/(2*dv)};
}
} // namespace kline
