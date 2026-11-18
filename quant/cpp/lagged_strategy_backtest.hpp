#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct LaggedStrategyStep{
    double active_weight=0,asset_return=0,turnover=0,
           transaction_fee=0,net_return=0,equity=0;
};
inline std::vector<LaggedStrategyStep> lagged_signal_strategy(
    const std::vector<double>& signals,
    const std::vector<double>& period_returns,
    double cost_per_unit,double initial_equity=1){
    check_finite(signals);check_finite(period_returns);
    require(signals.size()==period_returns.size()&&
            signals.size()>=2&&std::isfinite(cost_per_unit)&&
            cost_per_unit>=0&&std::isfinite(initial_equity)&&initial_equity>0,
            "Invalid causal strategy data");
    std::vector<LaggedStrategyStep> path;
    double last=0,equity=initial_equity;
    for(int t=0;t<int(signals.size());t++){
        double weight=t==0?0:signals[t-1];
        require(std::fabs(weight)<=1&&period_returns[t]>-1,
                "Invalid lagged position or return");
        double turnover=std::fabs(weight-last);
        double fee=turnover*cost_per_unit;
        double net=weight*period_returns[t]-fee;
        equity*=1+net;
        require(equity>0&&std::isfinite(equity),"Strategy equity exhausted");
        path.push_back({weight,period_returns[t],turnover,fee,net,equity});
        last=weight;
    }
    return path;
}
} // namespace kline
