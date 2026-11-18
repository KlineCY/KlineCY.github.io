#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline double amihud_illiquidity(
    const std::vector<double>& returns,const std::vector<double>& dollar_volume) {
    check_finite(returns);check_finite(dollar_volume);
    require(returns.size()==dollar_volume.size(),
            "Amihud arrays must match");
    double average=0;
    for(int i=0;i<int(returns.size());i++){
        require(dollar_volume[i]>0,"Amihud needs strictly positive turnover");
        average+=std::fabs(returns[i])/dollar_volume[i];
    }
    return average/returns.size();
}

inline double expected_square_root_impact(double turnover_fraction,
                                           double daily_volatility,double coefficient) {
    require(std::isfinite(turnover_fraction)&&turnover_fraction>=0&&
            std::isfinite(daily_volatility)&&daily_volatility>=0&&
            std::isfinite(coefficient)&&coefficient>=0,
            "Invalid market impact inputs");
    return coefficient*daily_volatility*std::sqrt(turnover_fraction);
}

inline double required_participation(double order_quantity,
                                     double market_quantity) {
    require(std::isfinite(order_quantity)&&order_quantity>=0&&
            std::isfinite(market_quantity)&&market_quantity>0,
            "Invalid trade participation inputs");
    return order_quantity/market_quantity;
}

inline double shares_from_notional(double notional,double price) {
    require(std::isfinite(notional)&&notional>=0&&
            std::isfinite(price)&&price>0,
            "Invalid notional-to-shares inputs");
    return notional/price;
}

inline double net_alpha_after_cost(double gross_expected_alpha,
                                   double turnover_fraction,
                                   double one_way_spread_bps,
                                   double commission_bps,
                                   double impact_bps) {
    require(std::isfinite(gross_expected_alpha)&&
            std::isfinite(turnover_fraction)&&turnover_fraction>=0&&
            std::isfinite(one_way_spread_bps)&&one_way_spread_bps>=0&&
            std::isfinite(commission_bps)&&commission_bps>=0&&
            std::isfinite(impact_bps)&&impact_bps>=0,
            "Invalid cost-adjusted alpha inputs");
    return gross_expected_alpha-
        turnover_fraction*(one_way_spread_bps+commission_bps+impact_bps)/10000;
}
} // namespace kline
