#pragma once
#include "derivatives.hpp"
#include <algorithm>
#include <cmath>

namespace kline {

inline double black76(double forward, double strike, double volatility,
                       double years, double discount_factor, bool call=true) {
    require(std::isfinite(forward) && std::isfinite(strike) &&
            forward > 0 && strike > 0 && std::isfinite(volatility) &&
            volatility >= 0 && std::isfinite(years) && years >= 0 &&
            std::isfinite(discount_factor) && discount_factor > 0,
            "Invalid Black-76 inputs");
    if (years == 0 || volatility == 0)
        return discount_factor*std::max(0.0,call ? forward-strike : strike-forward);
    double vol_time = volatility*std::sqrt(years);
    double d1 = std::log(forward/strike)/vol_time + 0.5*vol_time;
    double d2 = d1-vol_time;
    if (call)
        return discount_factor*(forward*standard_normal_cdf(d1) -
                                 strike*standard_normal_cdf(d2));
    return discount_factor*(strike*standard_normal_cdf(-d2) -
                            forward*standard_normal_cdf(-d1));
}

inline double bachelier_price(double forward, double strike,
                              double normal_volatility, double years,
                              double discount_factor, bool call=true) {
    require(std::isfinite(forward) && std::isfinite(strike) &&
            std::isfinite(normal_volatility) && normal_volatility >= 0 &&
            std::isfinite(years) && years >= 0 &&
            std::isfinite(discount_factor) && discount_factor > 0,
            "Invalid Bachelier inputs");
    double intrinsic = call ? forward-strike : strike-forward;
    if (normal_volatility == 0 || years == 0)
        return discount_factor*std::max(0.0,intrinsic);
    double deviation = normal_volatility*std::sqrt(years);
    double d = intrinsic/deviation;
    return discount_factor*(intrinsic*standard_normal_cdf(d) +
                            deviation*standard_normal_pdf(d));
}

inline double bachelier_implied_volatility(
    double market_price, double forward, double strike,
    double years, double discount_factor, bool call=true) {
    require(std::isfinite(market_price) && market_price >= 0 &&
            std::isfinite(years) && years > 0 &&
            std::isfinite(discount_factor) && discount_factor > 0,
            "Invalid implied normal-volatility inputs");
    double intrinsic = discount_factor*std::max(
        0.0,call?forward-strike:strike-forward);
    require(market_price > intrinsic, "Normal implied volatility needs time value");
    double left = 0, right = std::max(1.0,std::fabs(forward-strike));
    int expansions = 0;
    while (bachelier_price(forward,strike,right,years,discount_factor,call)
           < market_price && expansions++ < 100) right *= 2;
    require(expansions < 100 && std::isfinite(right),
            "Could not bracket normal implied volatility");
    for (int i = 0; i < 100; i++) {
        double mid = (left+right)/2;
        if (bachelier_price(forward,strike,mid,years,discount_factor,call)
            < market_price) left = mid;
        else right = mid;
    }
    return (left+right)/2;
}

inline double black76_vega(double forward, double strike, double volatility,
                            double years, double discount_factor) {
    require(forward > 0 && strike > 0 && volatility > 0 && years > 0 &&
            discount_factor > 0, "Invalid Black-76 vega inputs");
    double root = std::sqrt(years);
    double d1 = (std::log(forward/strike)+0.5*volatility*volatility*years)/
                (volatility*root);
    return discount_factor*forward*standard_normal_pdf(d1)*root;
}

} // namespace kline
