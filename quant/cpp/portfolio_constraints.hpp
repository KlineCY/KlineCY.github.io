#pragma once
#include "optimization.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
inline std::vector<double> enforce_gross_limit(
    const std::vector<double>& positions,double maximum_gross) {
    check_finite(positions);
    require(std::isfinite(maximum_gross)&&maximum_gross>=0,
            "Invalid gross limit");
    double gross=0;
    for(double w:positions)gross+=std::fabs(w);
    if(gross<=maximum_gross)return positions;
    std::vector<double> out=positions;
    for(double& value:out)value*=maximum_gross/gross;
    return out;
}

inline std::vector<double> dollar_neutral_signal(
    const std::vector<double>& signal,double target_gross=1) {
    check_finite(signal);
    require(signal.size()>=2&&std::isfinite(target_gross)&&target_gross>0,
            "Neutral portfolio needs two assets and positive gross");
    double center=mean(signal);
    std::vector<double> out=signal;
    double gross=0;
    for(double& value:out){
        value-=center;gross+=std::fabs(value);
    }
    require(gross>0,"Identical alpha signals cannot build a neutral position");
    for(double& value:out)value*=target_gross/gross;
    return out;
}

inline std::vector<double> rebalance_with_turnover_cap(
    const std::vector<double>& old_weights,
    const std::vector<double>& desired,double one_way_cap) {
    check_finite(old_weights);check_finite(desired);
    require(old_weights.size()==desired.size()&&
            std::isfinite(one_way_cap)&&one_way_cap>=0,
            "Invalid capped turnover inputs");
    double total=0;
    for(int i=0;i<int(desired.size());i++)
        total+=std::fabs(desired[i]-old_weights[i]);
    if(total<=2*one_way_cap)return desired;
    double fraction=(2*one_way_cap)/total;
    std::vector<double> out(old_weights.size());
    for(int i=0;i<int(out.size());i++)
        out[i]=old_weights[i]+fraction*(desired[i]-old_weights[i]);
    return out;
}

inline bool satisfies_net_gross(const std::vector<double>& weights,
                                double net_limit,double gross_limit) {
    check_finite(weights);
    require(std::isfinite(net_limit)&&net_limit>=0&&
            std::isfinite(gross_limit)&&gross_limit>=0,
            "Invalid net and gross constraints");
    double net=std::accumulate(weights.begin(),weights.end(),0.0);
    double gross=0;
    for(double x:weights)gross+=std::fabs(x);
    return std::fabs(net)<=net_limit+1e-10&&gross<=gross_limit+1e-10;
}
} // namespace kline
