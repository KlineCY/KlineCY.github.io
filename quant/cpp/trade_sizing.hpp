#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double fraction_kelly_binary(double success_probability,
    double payoff_multiple,double fractional=1){
    require(std::isfinite(success_probability)&&success_probability>=0&&
            success_probability<=1&&std::isfinite(payoff_multiple)&&
            payoff_multiple>0&&std::isfinite(fractional)&&fractional>=0&&
            fractional<=1,"Invalid binary Kelly parameters");
    return fractional*std::max(0.0,
        (success_probability*(payoff_multiple+1)-1)/payoff_multiple);
}
inline double normal_kelly_fraction(double expected_excess_return,
                                      double return_variance,double maximum){
    require(std::isfinite(expected_excess_return)&&
            std::isfinite(return_variance)&&return_variance>0&&
            std::isfinite(maximum)&&maximum>=0,"Invalid Gaussian Kelly");
    return std::max(-maximum,std::min(maximum,
                                    expected_excess_return/return_variance));
}
inline double volatility_scale_position(double target_volatility,
    double expected_volatility,double signal,double gross_cap){
    require(std::isfinite(target_volatility)&&target_volatility>=0&&
            std::isfinite(expected_volatility)&&expected_volatility>0&&
            std::isfinite(signal)&&std::isfinite(gross_cap)&&gross_cap>=0,
            "Invalid volatility-based size");
    return std::max(-gross_cap,std::min(gross_cap,
                signal*target_volatility/expected_volatility));
}
} // namespace kline
