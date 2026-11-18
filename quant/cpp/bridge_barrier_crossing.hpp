#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
inline double log_brownian_bridge_down_survival(
    double start_spot,double end_spot,double barrier,
    double volatility,double interval_years){
    require(std::isfinite(start_spot)&&std::isfinite(end_spot)&&
            std::isfinite(barrier)&&start_spot>0&&end_spot>0&&barrier>0&&
            std::isfinite(volatility)&&volatility>0&&
            std::isfinite(interval_years)&&interval_years>0,
            "Invalid Brownian bridge conditional survival");
    if(start_spot<=barrier||end_spot<=barrier)return 0;
    double a=std::log(start_spot/barrier),b=std::log(end_spot/barrier);
    return -std::expm1(-2*a*b/(volatility*volatility*interval_years));
}
inline double log_brownian_bridge_up_survival(
    double start_spot,double end_spot,double barrier,
    double volatility,double interval_years){
    require(std::isfinite(start_spot)&&std::isfinite(end_spot)&&
            std::isfinite(barrier)&&start_spot>0&&end_spot>0&&barrier>0&&
            std::isfinite(volatility)&&volatility>0&&
            std::isfinite(interval_years)&&interval_years>0,
            "Invalid up barrier bridge input");
    if(start_spot>=barrier||end_spot>=barrier)return 0;
    double a=std::log(barrier/start_spot),b=std::log(barrier/end_spot);
    return -std::expm1(-2*a*b/(volatility*volatility*interval_years));
}
} // namespace kline
