#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double maximum_call_vertical_violation(
   const std::vector<double>& strikes,const std::vector<double>& call_prices){
    check_finite(strikes);check_finite(call_prices);
    require(strikes.size()==call_prices.size()&&strikes.size()>=2,
            "Invalid call price strike grid");
    double violation=0;
    for(int i=1;i<int(strikes.size());i++){
        require(strikes[i]>strikes[i-1]&&call_prices[i]>=0&&
                call_prices[i-1]>=0,
                "Non-increasing strike or negative call price");
        violation=std::max(violation,call_prices[i]-call_prices[i-1]);
    }
    return violation;
}
inline double maximum_call_convexity_violation(
   const std::vector<double>& strikes,const std::vector<double>& prices){
    check_finite(strikes);check_finite(prices);
    require(strikes.size()==prices.size()&&strikes.size()>=3,
            "Invalid convexity strike grid");
    double violation=0;
    for(int i=1;i+1<int(strikes.size());i++){
        require(strikes[i-1]<strikes[i]&&strikes[i]<strikes[i+1],
                "Call strikes must be strictly ordered");
        double slope_left=(prices[i]-prices[i-1])/
                          (strikes[i]-strikes[i-1]);
        double slope_right=(prices[i+1]-prices[i])/
                           (strikes[i+1]-strikes[i]);
        violation=std::max(violation,slope_left-slope_right);
    }
    return violation;
}
} // namespace kline
