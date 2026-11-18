#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double inventory_variance_penalty(
    double stock_units,double volatility_fraction,double mid_price,
    double horizon_years,double risk_coefficient){
    require(std::isfinite(stock_units)&&
            std::isfinite(volatility_fraction)&&volatility_fraction>=0&&
            std::isfinite(mid_price)&&mid_price>0&&
            std::isfinite(horizon_years)&&horizon_years>=0&&
            std::isfinite(risk_coefficient)&&risk_coefficient>=0,
            "Invalid inventory uncertainty inputs");
    double money_risk=stock_units*mid_price*volatility_fraction;
    return 0.5*risk_coefficient*money_risk*money_risk*horizon_years;
}
inline double inventory_mark_to_market_pnl(
    double stock_units,double prior_mid,double current_mid){
    require(std::isfinite(stock_units)&&
            std::isfinite(prior_mid)&&prior_mid>0&&
            std::isfinite(current_mid)&&current_mid>0,
            "Invalid inventory mark to market");
    return stock_units*(current_mid-prior_mid);
}
} // namespace kline
