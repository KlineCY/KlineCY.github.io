#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double parity_forward_component(double spot,double strike,double rate,
                                       double dividend_yield,double maturity){
    require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(rate)&&std::isfinite(dividend_yield)&&
            std::isfinite(maturity)&&maturity>=0,
            "Invalid European parity contract");
    return spot*std::exp(-dividend_yield*maturity)-
           strike*std::exp(-rate*maturity);
}
inline double call_put_parity_deviation(double call,double put,
    double spot,double strike,double rate,double yield,double maturity){
    require(std::isfinite(call)&&call>=0&&std::isfinite(put)&&put>=0,
            "Negative option market quote");
    return call-put-parity_forward_component(spot,strike,rate,yield,maturity);
}
} // namespace kline
