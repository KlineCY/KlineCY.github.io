#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>

namespace kline {
inline double reservation_price(double mid,double inventory,
                                 double risk_aversion,double variance,
                                 double horizon) {
    require(std::isfinite(mid)&&mid>0&&std::isfinite(inventory)&&
            std::isfinite(risk_aversion)&&risk_aversion>=0&&
            std::isfinite(variance)&&variance>=0&&
            std::isfinite(horizon)&&horizon>=0,
            "Invalid inventory reservation price inputs");
    return mid-inventory*risk_aversion*variance*horizon;
}

inline double optimal_market_making_spread(
    double risk_aversion,double variance,double horizon,
    double arrival_exponent) {
    require(std::isfinite(risk_aversion)&&risk_aversion>0&&
            std::isfinite(variance)&&variance>=0&&
            std::isfinite(horizon)&&horizon>=0&&
            std::isfinite(arrival_exponent)&&arrival_exponent>0,
            "Invalid stylized market-making parameters");
    return risk_aversion*variance*horizon+
           2*std::log1p(risk_aversion/arrival_exponent)/risk_aversion;
}

inline double bid_inventory_quote(double mid,double inventory,
                                   double gamma,double variance,
                                   double horizon,double arrival_exponent) {
    return reservation_price(mid,inventory,gamma,variance,horizon)-
           optimal_market_making_spread(gamma,variance,horizon,
                                        arrival_exponent)/2;
}

inline double ask_inventory_quote(double mid,double inventory,
                                   double gamma,double variance,
                                   double horizon,double arrival_exponent) {
    return reservation_price(mid,inventory,gamma,variance,horizon)+
           optimal_market_making_spread(gamma,variance,horizon,
                                        arrival_exponent)/2;
}

inline double quoted_inventory_skew(double bid,double ask,double mid) {
    require(std::isfinite(bid)&&std::isfinite(ask)&&std::isfinite(mid)&&
            mid>0&&bid<=ask,"Invalid inventory quote");
    return (bid+ask)/2-mid;
}
} // namespace kline
