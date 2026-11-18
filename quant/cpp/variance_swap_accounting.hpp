#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double annualized_realized_variance(
    const std::vector<double>& log_returns,int periods_per_year=252){
    check_finite(log_returns);
    require(periods_per_year>0,"Invalid variance swap convention");
    double sum=0;
    for(double r:log_returns)sum+=r*r;
    return periods_per_year*sum/log_returns.size();
}
inline double variance_swap_settlement(double realized_variance,
             double strike_variance,double variance_notional){
    require(std::isfinite(realized_variance)&&realized_variance>=0&&
            std::isfinite(strike_variance)&&strike_variance>=0&&
            std::isfinite(variance_notional),
            "Invalid variance swap settlement");
    return variance_notional*(realized_variance-strike_variance);
}
inline double vega_notional_to_variance_notional(double vega_notional,
                                                 double strike_volatility){
    require(std::isfinite(vega_notional)&&std::isfinite(strike_volatility)&&
            strike_volatility>0,"Variance notional conversion");
    return vega_notional/(2*strike_volatility);
}
} // namespace kline
