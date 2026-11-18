#pragma once
#include "derivatives.hpp"
#include <cmath>
namespace kline {
struct Black76Greeks{double value=0,forward_delta=0,forward_gamma=0,vega=0;};
inline Black76Greeks black76_futures_option(
    double forward,double strike,double rate,double volatility,
    double maturity,bool call){
    require(std::isfinite(forward)&&forward>0&&std::isfinite(strike)&&
            strike>0&&std::isfinite(rate)&&std::isfinite(volatility)&&
            volatility>0&&std::isfinite(maturity)&&maturity>0,
            "Invalid Black 76 futures option");
    double standard=volatility*std::sqrt(maturity);
    double d1=(std::log(forward/strike)+0.5*standard*standard)/standard;
    double d2=d1-standard,df=std::exp(-rate*maturity);
    double call_value=df*(forward*standard_normal_cdf(d1)-
                          strike*standard_normal_cdf(d2));
    double put_value=call_value-df*(forward-strike);
    double delta=df*(call?standard_normal_cdf(d1):
                           standard_normal_cdf(d1)-1);
    return {call?call_value:put_value,delta,
            df*standard_normal_pdf(d1)/(forward*standard),
            df*forward*standard_normal_pdf(d1)*std::sqrt(maturity)};
}
} // namespace kline
