#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double cross_rate_multiplication(double domestic_per_base,
                                       double quote_per_domestic){
    require(std::isfinite(domestic_per_base)&&domestic_per_base>0&&
            std::isfinite(quote_per_domestic)&&quote_per_domestic>0,
            "Invalid FX cross quotation");
    return domestic_per_base*quote_per_domestic;
}
inline double triangular_log_mispricing(
    double domestic_per_base,double quote_per_domestic,
    double quote_per_base){
    require(std::isfinite(quote_per_base)&&quote_per_base>0,
            "Invalid FX direct quotation");
    return std::log(cross_rate_multiplication(
        domestic_per_base,quote_per_domestic)/quote_per_base);
}
} // namespace kline
