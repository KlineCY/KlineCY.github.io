#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
inline double binary_kelly_fraction(double probability_win,
                                    double gain_on_win,double loss_on_lose){
    require(std::isfinite(probability_win)&&probability_win>=0&&
            probability_win<=1&&std::isfinite(gain_on_win)&&gain_on_win>0&&
            std::isfinite(loss_on_lose)&&loss_on_lose>0,
            "Invalid Kelly binary contract");
    double unconstrained=probability_win/loss_on_lose-
                         (1-probability_win)/gain_on_win;
    return std::max(0.0,std::min(1/loss_on_lose,unconstrained));
}
inline double fractional_kelly_fraction(double probability_win,
                double gain_on_win,double loss_on_lose,double shrink_fraction){
    require(std::isfinite(shrink_fraction)&&shrink_fraction>=0&&
            shrink_fraction<=1,"Fractional Kelly reduction outside [0,1]");
    return shrink_fraction*binary_kelly_fraction(
        probability_win,gain_on_win,loss_on_lose);
}
} // namespace kline
