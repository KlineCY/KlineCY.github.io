#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> time_aggregated_nonoverlap_returns(
    const std::vector<double>& simple_returns,int periods){
    check_finite(simple_returns);
    require(periods>=1&&periods<=int(simple_returns.size()),
            "Invalid aggregation horizon");
    std::vector<double> output;
    for(int i=0;i+periods<=int(simple_returns.size());i+=periods){
        double wealth=1;
        for(int j=i;j<i+periods;j++){
            require(simple_returns[j]>-1,
                    "Simple return below -100 percent");
            wealth*=1+simple_returns[j];
        }
        output.push_back(wealth-1);
    }
    return output;
}
inline std::vector<double> rolling_bipower_variation(
    const std::vector<double>& log_returns,int window){
    check_finite(log_returns);
    require(window>=3&&window<=int(log_returns.size()),
            "Insufficient return window for bipower variation");
    const double scale=std::acos(-1.0)/2;
    std::vector<double> result;
    for(int end=window;end<=int(log_returns.size());end++){
        double sum=0;
        for(int t=end-window+1;t<end;t++)
            sum+=std::fabs(log_returns[t])*std::fabs(log_returns[t-1]);
        result.push_back(scale*sum*double(window)/(window-1));
    }
    return result;
}
} // namespace kline
