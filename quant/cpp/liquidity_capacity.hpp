#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> capacity_constrained_shares(
    const std::vector<double>& desired_shares,
    const std::vector<double>& daily_volume,double participation){
    check_finite(desired_shares);check_finite(daily_volume);
    require(desired_shares.size()==daily_volume.size()&&
            std::isfinite(participation)&&participation>0&&participation<=1,
            "Invalid liquidity capacity inputs");
    std::vector<double> approved;
    for(int i=0;i<int(desired_shares.size());i++){
        require(daily_volume[i]>=0,"Negative expected volume");
        double cap=participation*daily_volume[i];
        approved.push_back(std::max(-cap,std::min(cap,desired_shares[i])));
    }
    return approved;
}
inline double participation_weighted_execution_time(
    const std::vector<double>& desired_shares,
    const std::vector<double>& daily_volume,double participation){
    auto allowed=capacity_constrained_shares(
        desired_shares,daily_volume,participation);
    double days=0;
    for(int i=0;i<int(allowed.size());i++){
        if(desired_shares[i]!=0){
            require(allowed[i]!=0,"No executable liquidity");
            days=std::max(days,std::fabs(desired_shares[i]/allowed[i]));
        }
    }
    return days;
}
} // namespace kline
