#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
namespace kline {
inline std::vector<double> drifted_portfolio_weights(
    const std::vector<double>& initial_weights,
    const std::vector<double>& period_returns,double cash_weight=0){
    check_finite(initial_weights);check_finite(period_returns);
    require(initial_weights.size()==period_returns.size()&&
            std::isfinite(cash_weight)&&cash_weight>=0,
            "Invalid portfolio drift weights");
    double assets=cash_weight;
    std::vector<double> next;
    for(int i=0;i<int(initial_weights.size());i++){
        require(initial_weights[i]>=0&&period_returns[i]>-1,
                "Long only drift requires nonnegative weights and viable returns");
        next.push_back(initial_weights[i]*(1+period_returns[i]));
        assets+=next.back();
    }
    require(assets>0,"Drifted portfolio has no value");
    for(double& v:next)v/=assets;
    return next;
}
inline std::vector<double> no_trade_band_rebalance(
    const std::vector<double>& current,
    const std::vector<double>& target,double maximum_deviation){
    check_finite(current);check_finite(target);
    require(current.size()==target.size()&&
            std::isfinite(maximum_deviation)&&maximum_deviation>=0,
            "Invalid no trade band portfolio");
    for(int i=0;i<int(current.size());i++)
        require(current[i]>=0&&target[i]>=0,
                "Rebalancing rule uses nonnegative long-only weights");
    bool needed=false;
    for(int i=0;i<int(current.size());i++)
        needed|=std::fabs(target[i]-current[i])>maximum_deviation;
    return needed?target:current;
}
} // namespace kline
