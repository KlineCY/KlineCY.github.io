#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double dupire_local_volatility_squared(
    double strike,double call_price,double time_derivative,
    double strike_derivative,double strike_second_derivative,
    double rate,double dividend_yield){
    require(std::isfinite(strike)&&strike>0&&std::isfinite(call_price)&&
            call_price>=0&&std::isfinite(time_derivative)&&
            std::isfinite(strike_derivative)&&
            std::isfinite(strike_second_derivative)&&
            strike_second_derivative>0&&std::isfinite(rate)&&
            std::isfinite(dividend_yield),"Invalid Dupire local-vol inputs");
    double numerator=time_derivative+(rate-dividend_yield)*
                     strike*strike_derivative+dividend_yield*call_price;
    double denominator=0.5*strike*strike*strike_second_derivative;
    require(numerator>=0,"Negative Dupire local variance numerator");
    return numerator/denominator;
}
} // namespace kline
