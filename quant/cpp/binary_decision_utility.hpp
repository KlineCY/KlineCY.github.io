#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double optimal_probability_threshold(
    double gain_if_correct,double loss_if_incorrect,double trading_cost){
    require(std::isfinite(gain_if_correct)&&gain_if_correct>0&&
            std::isfinite(loss_if_incorrect)&&loss_if_incorrect>=0&&
            std::isfinite(trading_cost)&&trading_cost>=0&&
            trading_cost<gain_if_correct,"Invalid binary payoff inputs");
    return (loss_if_incorrect+trading_cost)/
           (gain_if_correct+loss_if_incorrect);
}
inline double expected_trade_utility(double probability,
    double gain_if_correct,double loss_if_incorrect,double trading_cost){
    require(std::isfinite(probability)&&probability>=0&&probability<=1&&
            std::isfinite(gain_if_correct)&&gain_if_correct>=0&&
            std::isfinite(loss_if_incorrect)&&loss_if_incorrect>=0&&
            std::isfinite(trading_cost)&&trading_cost>=0,
            "Invalid binary expected trade utility inputs");
    return probability*gain_if_correct-(1-probability)*loss_if_incorrect-
           trading_cost;
}
} // namespace kline
