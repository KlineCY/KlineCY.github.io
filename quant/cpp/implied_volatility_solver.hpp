#pragma once
#include "derivatives.hpp"
#include <cmath>
namespace kline {
inline double implied_black_scholes_volatility(
    double option_price,double spot,double strike,double rate,
    double dividend_yield,double maturity,bool call){
    require(std::isfinite(option_price)&&option_price>=0&&
            std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(rate)&&std::isfinite(dividend_yield)&&
            std::isfinite(maturity)&&maturity>0,"Invalid implied vol contract");
    double dfq=std::exp(-dividend_yield*maturity),dfr=std::exp(-rate*maturity);
    double intrinsic=call?std::max(spot*dfq-strike*dfr,0.0):
                          std::max(strike*dfr-spot*dfq,0.0);
    double upper=call?spot*dfq:strike*dfr;
    require(option_price>intrinsic+1e-12&&option_price<upper-1e-12,
            "Option quote outside strict implied-volatility bounds");
    double lo=1e-10,hi=0.5;
    for(int repeat=0;repeat<30;repeat++){
        if(black_scholes(spot,strike,rate,dividend_yield,hi,maturity,call).value
           >=option_price)break;
        hi*=2;
    }
    require(black_scholes(spot,strike,rate,dividend_yield,hi,maturity,call).value
             >=option_price,"Implied volatility upper search failed");
    for(int repeat=0;repeat<100;repeat++){
        double mid=(lo+hi)/2;
        double model=black_scholes(spot,strike,rate,dividend_yield,
                                   mid,maturity,call).value;
        if(model<option_price)lo=mid;
        else hi=mid;
    }
    return (hi+lo)/2;
}
} // namespace kline
