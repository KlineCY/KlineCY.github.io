#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct RegimeComparison{
    double mean_first=0,mean_second=0;
    double volatility_first=0,volatility_second=0;
    double mean_difference=0;
};
inline RegimeComparison compare_market_regimes(
    const std::vector<double>& returns,int breakpoint){
    check_finite(returns);
    require(breakpoint>=3&&breakpoint+3<=int(returns.size()),
            "Regimes need three observations each");
    std::vector<double> first(returns.begin(),returns.begin()+breakpoint);
    std::vector<double> second(returns.begin()+breakpoint,returns.end());
    RegimeComparison r;
    r.mean_first=mean(first);r.mean_second=mean(second);
    r.volatility_first=std::sqrt(variance(first));
    r.volatility_second=std::sqrt(variance(second));
    r.mean_difference=r.mean_second-r.mean_first;
    return r;
}
inline double cumulative_sum_change_score(
    const std::vector<double>& observations){
    check_finite(observations);
    require(observations.size()>=3,"CUSUM score needs three samples");
    double center=mean(observations),total=0,largest=0;
    for(double x:observations){
        total+=x-center;
        largest=std::max(largest,std::fabs(total));
    }
    double sigma=std::sqrt(variance(observations,false));
    require(sigma>0,"CUSUM change score undefined for constant data");
    return largest/(sigma*std::sqrt(double(observations.size())));
}
} // namespace kline
