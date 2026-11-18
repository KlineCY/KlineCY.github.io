#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double square_root_impact_fraction(
    double daily_volatility,double order_shares,
    double expected_daily_volume,double impact_coefficient){
    require(std::isfinite(daily_volatility)&&daily_volatility>=0&&
            std::isfinite(order_shares)&&order_shares>=0&&
            std::isfinite(expected_daily_volume)&&expected_daily_volume>0&&
            std::isfinite(impact_coefficient)&&impact_coefficient>=0,
            "Invalid square-root market impact");
    return impact_coefficient*daily_volatility*
           std::sqrt(order_shares/expected_daily_volume);
}
inline double estimated_implementation_cost(
    double order_notional,double half_spread,
    double impact_fraction,double commission_fraction){
    require(std::isfinite(order_notional)&&order_notional>=0&&
            std::isfinite(half_spread)&&half_spread>=0&&
            std::isfinite(impact_fraction)&&impact_fraction>=0&&
            std::isfinite(commission_fraction)&&commission_fraction>=0,
            "Invalid predicted execution cost");
    return order_notional*
           (half_spread+impact_fraction+commission_fraction);
}
} // namespace kline
