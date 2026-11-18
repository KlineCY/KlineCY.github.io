#pragma once
#include "fx_forward_parity.hpp"
#include <cmath>
namespace kline {
inline double hedged_foreign_investment_terminal(
    double foreign_notional,double future_foreign_value,
    double spot_domestic_per_foreign,double forward_domestic_per_foreign,
    double hedge_ratio){
    require(std::isfinite(foreign_notional)&&foreign_notional>=0&&
            std::isfinite(future_foreign_value)&&future_foreign_value>=0&&
            std::isfinite(spot_domestic_per_foreign)&&spot_domestic_per_foreign>0&&
            std::isfinite(forward_domestic_per_foreign)&&
            forward_domestic_per_foreign>0&&
            std::isfinite(hedge_ratio)&&hedge_ratio>=0&&hedge_ratio<=1,
            "Invalid FX hedge contract");
    return future_foreign_value*spot_domestic_per_foreign+
           hedge_ratio*foreign_notional*
           (forward_domestic_per_foreign-spot_domestic_per_foreign);
}
inline double hedged_foreign_investment_return(
    double initial_domestic_value,double terminal_domestic_value){
    require(std::isfinite(initial_domestic_value)&&initial_domestic_value>0&&
            std::isfinite(terminal_domestic_value),
            "Invalid FX hedged return denominator");
    return terminal_domestic_value/initial_domestic_value-1;
}
} // namespace kline
