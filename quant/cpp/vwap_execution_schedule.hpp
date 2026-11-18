#pragma once
#include "stats.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<double> volume_profile_schedule(
    double quantity,const std::vector<double>& forecast_volume){
    check_finite(forecast_volume);
    require(std::isfinite(quantity)&&quantity>=0,
            "Invalid VWAP order quantity");
    double sum=0;
    for(double v:forecast_volume){
        require(v>=0,"Invalid volume profile");sum+=v;
    }
    require(sum>0,"VWAP needs positive forecast volume");
    std::vector<double> schedule;
    for(double v:forecast_volume)schedule.push_back(quantity*v/sum);
    return schedule;
}
inline double executed_vwap(const std::vector<double>& prices,
                            const std::vector<double>& filled){
    check_finite(prices);check_finite(filled);
    require(prices.size()==filled.size(),"VWAP fill and price shape");
    double value=0,total=0;
    for(int i=0;i<int(prices.size());i++){
        require(prices[i]>0&&filled[i]>=0,"Invalid VWAP execution input");
        value+=prices[i]*filled[i];total+=filled[i];
    }
    require(total>0,"No filled shares for VWAP");
    return value/total;
}
} // namespace kline
