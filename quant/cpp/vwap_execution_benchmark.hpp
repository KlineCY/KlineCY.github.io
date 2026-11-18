#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double market_volume_weighted_price(
    const std::vector<double>& prices,const std::vector<double>& volumes){
    check_finite(prices);check_finite(volumes);
    require(prices.size()==volumes.size(),"VWAP prices and volumes misaligned");
    double total_vol=0,total_notional=0;
    for(int i=0;i<int(prices.size());i++){
        require(prices[i]>0&&volumes[i]>=0,"Invalid VWAP trade");
        total_vol+=volumes[i];
        total_notional+=prices[i]*volumes[i];
    }
    require(total_vol>0,"VWAP needs nonzero traded volume");
    return total_notional/total_vol;
}
inline double vwap_slippage_fraction(
    double execution_price,double benchmark,bool buy){
    require(std::isfinite(execution_price)&&execution_price>0&&
            std::isfinite(benchmark)&&benchmark>0,
            "Invalid VWAP execution benchmark");
    return (buy?1:-1)*(execution_price-benchmark)/benchmark;
}
} // namespace kline
