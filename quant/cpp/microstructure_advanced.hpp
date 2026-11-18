#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline double roll_effective_spread(const std::vector<double>& prices) {
    check_finite(prices);
    require(prices.size()>=4,"Roll spread needs four trades");
    std::vector<double> changes;
    for(int t=1;t<int(prices.size());t++)
        changes.push_back(prices[t]-prices[t-1]);
    std::vector<double> left(changes.begin()+1,changes.end());
    std::vector<double> right(changes.begin(),changes.end()-1);
    double autocov=covariance(left,right,false);
    return autocov<0?2*std::sqrt(-autocov):0;
}

inline double order_imbalance(const std::vector<double>& signed_quantities) {
    check_finite(signed_quantities);
    double gross=0,net=0;
    for(double value:signed_quantities){
        gross+=std::fabs(value);net+=value;
    }
    require(gross>0,"Order imbalance needs positive volume");
    return net/gross;
}

inline double kyle_lambda(const std::vector<double>& price_changes,
                          const std::vector<double>& signed_volume) {
    check_finite(price_changes);check_finite(signed_volume);
    require(price_changes.size()==signed_volume.size()&&
            price_changes.size()>=3,"Kyle regression needs aligned observations");
    double v=variance(signed_volume,false);
    require(v>0,"Kyle regression needs variable signed volume");
    return covariance(price_changes,signed_volume,false)/v;
}

inline double realized_variance(const std::vector<double>& log_returns) {
    check_finite(log_returns);
    double sum=0;
    for(double value:log_returns)sum+=value*value;
    return sum;
}

inline double bipower_variation(const std::vector<double>& log_returns) {
    check_finite(log_returns);
    require(log_returns.size()>=2,"Bipower needs two returns");
    double sum=0;
    for(int t=1;t<int(log_returns.size());t++)
        sum+=std::fabs(log_returns[t])*std::fabs(log_returns[t-1]);
    return (acos(-1.0)/2)*sum;
}

inline double realized_jump_variation(const std::vector<double>& log_returns) {
    return std::max(0.0,realized_variance(log_returns)-
                        bipower_variation(log_returns));
}
} // namespace kline
