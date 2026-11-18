#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double exponential_fill_intensity(double baseline,double exponent,
                                         double quote_distance){
    require(std::isfinite(baseline)&&baseline>=0&&
            std::isfinite(exponent)&&exponent>0&&
            std::isfinite(quote_distance)&&quote_distance>=0,
            "Invalid queue arrival intensity");
    return baseline*std::exp(-exponent*quote_distance);
}
inline double poisson_arrival_fill_probability(double arrival_intensity,
                                                double duration){
    require(std::isfinite(arrival_intensity)&&arrival_intensity>=0&&
            std::isfinite(duration)&&duration>=0,
            "Invalid Poisson fill inputs");
    return -std::expm1(-arrival_intensity*duration);
}
inline double queue_depletion_probability(double expected_market_orders,
                                          double queue_ahead){
    require(std::isfinite(expected_market_orders)&&expected_market_orders>=0&&
            std::isfinite(queue_ahead)&&queue_ahead>=0,
            "Invalid stylized queue depletion inputs");
    return 1-std::exp(-expected_market_orders/(queue_ahead+1));
}
} // namespace kline
