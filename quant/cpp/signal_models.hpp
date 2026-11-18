#pragma once
#include "timeseries.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline std::vector<double> time_series_trend(
    const std::vector<double>& prices,int fast,int slow) {
    check_finite(prices);
    require(fast>0&&slow>fast,"Trend EMA spans must be ordered");
    for(double p:prices)require(p>0,"Trend needs positive prices");
    auto quick=exponential_mean(prices,2.0/(fast+1));
    auto long_term=exponential_mean(prices,2.0/(slow+1));
    std::vector<double> result;
    for(int i=0;i<int(prices.size());i++)
        result.push_back(quick[i]/long_term[i]-1);
    return result;
}

inline std::vector<double> lagged_position_from_threshold(
    const std::vector<double>& signal,double entry,
    double exit) {
    check_finite(signal);
    require(std::isfinite(entry)&&std::isfinite(exit)&&
            entry>0&&exit>=0&&exit<=entry,
            "Hysteresis thresholds are invalid");
    std::vector<double> positions(signal.size(),0);
    double position=0;
    for(int t=1;t<int(signal.size());t++){
        double prior=signal[t-1];
        if(position==0){
            if(prior>=entry)position=1;
            else if(prior<=-entry)position=-1;
        }else if(position==1){
            if(prior<=-entry)position=-1;
            else if(prior<exit)position=0;
        }else{
            if(prior>=entry)position=1;
            else if(prior>-exit)position=0;
        }
        positions[t]=position;
    }
    return positions;
}

inline std::vector<double> strategy_returns_from_lagged_positions(
    const std::vector<double>& positions,
    const std::vector<double>& subsequent_returns,
    double cost_per_unit=0) {
    check_finite(positions);check_finite(subsequent_returns);
    require(positions.size()==subsequent_returns.size()&&
            std::isfinite(cost_per_unit)&&cost_per_unit>=0,
            "Invalid aligned position returns");
    std::vector<double> out;
    double previous=0;
    for(int t=0;t<int(positions.size());t++){
        require(std::fabs(positions[t])<=1,"Position exceeds unit exposure");
        out.push_back(positions[t]*subsequent_returns[t]-
                      cost_per_unit*std::fabs(positions[t]-previous));
        previous=positions[t];
    }
    return out;
}

inline std::vector<double> rolling_zscore_past_only(
    const std::vector<double>& values,int lookback) {
    check_finite(values);
    require(lookback>=2&&lookback<int(values.size()),
            "Rolling z-score needs historical observations");
    std::vector<double> output;
    for(int t=lookback;t<int(values.size());t++){
        std::vector<double> past(values.begin()+t-lookback,values.begin()+t);
        double sigma=std::sqrt(variance(past,false));
        output.push_back(sigma>0?(values[t]-mean(past))/sigma:0);
    }
    return output;
}
} // namespace kline
