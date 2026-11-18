#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct ParticipationPlan{std::vector<double> fills;double unfilled=0;};
inline ParticipationPlan volume_limited_execution(
    double total_shares,const std::vector<double>& observed_market_volume,
    double participation){
    check_finite(observed_market_volume);
    require(std::isfinite(total_shares)&&total_shares>=0&&
            std::isfinite(participation)&&participation>0&&participation<=1,
            "Invalid volume participation plan");
    ParticipationPlan plan;plan.unfilled=total_shares;
    for(double volume:observed_market_volume){
        require(volume>=0,"Negative observed market volume");
        double traded=std::min(plan.unfilled,participation*volume);
        plan.fills.push_back(traded);
        plan.unfilled-=traded;
    }
    return plan;
}
inline double realized_participation_ratio(
    const std::vector<double>& traded,const std::vector<double>& market_volume){
    check_finite(traded);check_finite(market_volume);
    require(traded.size()==market_volume.size(),"Participation volume shape");
    double executed=0,market=0;
    for(int i=0;i<int(traded.size());i++){
        require(traded[i]>=0&&market_volume[i]>=0,"Invalid participation trades");
        executed+=traded[i];market+=market_volume[i];
    }
    require(market>0,"Market volume is zero");
    return executed/market;
}
} // namespace kline
