#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double covered_fx_forward(
    double spot_base_per_foreign,double domestic_rate,
    double foreign_rate,double years){
    require(std::isfinite(spot_base_per_foreign)&&spot_base_per_foreign>0&&
            std::isfinite(domestic_rate)&&std::isfinite(foreign_rate)&&
            std::isfinite(years)&&years>=0,
            "Invalid covered interest parity input");
    return spot_base_per_foreign*
           std::exp((domestic_rate-foreign_rate)*years);
}
inline double fx_covered_basis(
    double observed_forward,double spot,double domestic_rate,
    double foreign_rate,double years){
    require(std::isfinite(observed_forward)&&observed_forward>0&&
            std::isfinite(years)&&years>0,"Invalid FX covered basis");
    double parity=covered_fx_forward(spot,domestic_rate,foreign_rate,years);
    return std::log(observed_forward/parity)/years;
}
} // namespace kline
