#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double black_scholes_cash_digital_call(
 double spot,double strike,double rate,double dividend,double sigma,double maturity){
 require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
         std::isfinite(rate)&&std::isfinite(dividend)&&
         std::isfinite(sigma)&&sigma>0&&std::isfinite(maturity)&&maturity>0,
         "Invalid cash-or-nothing option");
 const double d2=(std::log(spot/strike)+(rate-dividend-0.5*sigma*sigma)*maturity)/
                 (sigma*std::sqrt(maturity));
 return std::exp(-rate*maturity)*0.5*std::erfc(-d2/std::sqrt(2.0));
}
inline double black_scholes_vanna_call(
 double spot,double strike,double rate,double dividend,double sigma,double maturity){
 require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
         std::isfinite(rate)&&std::isfinite(dividend)&&
         std::isfinite(sigma)&&sigma>0&&std::isfinite(maturity)&&maturity>0,
         "Invalid vanna option");
 const double d1=(std::log(spot/strike)+(rate-dividend+0.5*sigma*sigma)*maturity)/
                 (sigma*std::sqrt(maturity));
 const double d2=d1-sigma*std::sqrt(maturity);
 return -std::exp(-dividend*maturity-d1*d1/2)/std::sqrt(2*std::acos(-1.0))*
        d2/sigma;
}
inline double black_scholes_volga_call(
 double spot,double strike,double rate,double dividend,double sigma,double maturity){
 require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
         std::isfinite(rate)&&std::isfinite(dividend)&&
         std::isfinite(sigma)&&sigma>0&&std::isfinite(maturity)&&maturity>0,
         "Invalid volga option");
 const double d1=(std::log(spot/strike)+(rate-dividend+0.5*sigma*sigma)*maturity)/
                 (sigma*std::sqrt(maturity));
 const double d2=d1-sigma*std::sqrt(maturity);
 const double vega=spot*std::exp(-dividend*maturity-d1*d1/2)*
                   std::sqrt(maturity/(2*std::acos(-1.0)));
 return vega*d1*d2/sigma;
}
inline double annualized_forward_variance(
 double nearYears,double nearVol,double farYears,double farVol){
 require(std::isfinite(nearYears)&&nearYears>=0&&
         std::isfinite(farYears)&&farYears>nearYears&&
         std::isfinite(nearVol)&&nearVol>=0&&
         std::isfinite(farVol)&&farVol>=0,
         "Invalid implied volatility maturities");
 const double diff=farYears*farVol*farVol-nearYears*nearVol*nearVol;
 require(diff>=-1e-12,"Calendar total variance decreases");
 return std::max(0.0,diff)/(farYears-nearYears);
}
}
