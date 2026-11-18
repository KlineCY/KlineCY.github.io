#pragma once
#include "derivatives.hpp"
#include <cmath>
namespace kline {
struct BachelierPrice{double value=0,delta_forward=0,vega_normal=0;};
inline BachelierPrice bachelier_option(
     double forward,double strike,double rate,double normal_vol,
     double maturity,bool call){
    require(std::isfinite(forward)&&std::isfinite(strike)&&
            std::isfinite(rate)&&std::isfinite(normal_vol)&&normal_vol>0&&
            std::isfinite(maturity)&&maturity>0,
            "Invalid Bachelier normal option");
    double deviation=normal_vol*std::sqrt(maturity);
    double d=(forward-strike)/deviation,df=std::exp(-rate*maturity);
    double call_value=df*((forward-strike)*standard_normal_cdf(d)+
                          deviation*standard_normal_pdf(d));
    double price=call?call_value:call_value-df*(forward-strike);
    return {price,df*(call?standard_normal_cdf(d):
                            standard_normal_cdf(d)-1),
            df*std::sqrt(maturity)*standard_normal_pdf(d)};
}
} // namespace kline
