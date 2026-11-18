#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double call_implied_probability_above_strike(
    double call_price_below,double call_price_above,
    double strike_below,double strike_above,double discount){
    require(std::isfinite(call_price_below)&&call_price_below>=0&&
            std::isfinite(call_price_above)&&call_price_above>=0&&
            std::isfinite(strike_below)&&std::isfinite(strike_above)&&
            strike_above>strike_below&&
            std::isfinite(discount)&&discount>0,
            "Invalid digital option call spread inputs");
    double digital=(call_price_below-call_price_above)/
                   ((strike_above-strike_below)*discount);
    require(digital>=-1e-9&&digital<=1+1e-9,
            "Call prices violate monotone spread probability bound");
    return std::max(0.0,std::min(1.0,digital));
}
inline double call_implied_density_equal_spacing(
    double call_low,double call_mid,double call_high,
    double strike_step,double discount){
    require(std::isfinite(call_low)&&call_low>=0&&
            std::isfinite(call_mid)&&call_mid>=0&&
            std::isfinite(call_high)&&call_high>=0&&
            std::isfinite(strike_step)&&strike_step>0&&
            std::isfinite(discount)&&discount>0,
            "Invalid option density finite difference");
    double density=(call_low-2*call_mid+call_high)/
                   (discount*strike_step*strike_step);
    require(density>=-1e-10,"Negative risk neutral density proxy");
    return std::max(0.0,density);
}
} // namespace kline
