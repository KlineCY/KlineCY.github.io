#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double herfindahl_concentration(const std::vector<double>& weights){
    check_finite(weights);
    double sum=0,hhi=0;
    for(double w:weights){
        require(w>=0,"Concentration requires nonnegative shares");
        sum+=w;hhi+=w*w;
    }
    require(sum>0,"Concentration weight mass zero");
    return hhi/(sum*sum);
}
inline double effective_holdings_count(const std::vector<double>& weights){
    return 1/herfindahl_concentration(weights);
}
} // namespace kline
