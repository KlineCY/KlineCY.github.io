#pragma once
#include "options_models.hpp"
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double total_implied_variance(double volatility,double years) {
    require(std::isfinite(volatility)&&volatility>=0&&
            std::isfinite(years)&&years>=0,
            "Invalid total variance inputs");
    return volatility*volatility*years;
}

inline double variance_forward_rate(double shorter_years,double short_vol,
                                    double longer_years,double long_vol) {
    require(std::isfinite(shorter_years)&&shorter_years>=0&&
            std::isfinite(longer_years)&&longer_years>shorter_years,
            "Variance maturities must increase");
    double forward=(total_implied_variance(long_vol,longer_years)-
                    total_implied_variance(short_vol,shorter_years))/
                   (longer_years-shorter_years);
    require(forward>=-1e-12,
            "Total variance calendar inversion detected");
    return std::max(0.0,forward);
}

inline double call_vertical_spread(double lower_strike_call,
                                    double higher_strike_call) {
    require(std::isfinite(lower_strike_call)&&
            std::isfinite(higher_strike_call),
            "Invalid call vertical spread premiums");
    return lower_strike_call-higher_strike_call;
}

inline double butterfly_call_cost(double call_at_lower_strike,
                                   double call_at_mid_strike,
                                   double call_at_upper_strike) {
    require(std::isfinite(call_at_lower_strike)&&
            std::isfinite(call_at_mid_strike)&&
            std::isfinite(call_at_upper_strike),
            "Invalid butterfly premiums");
    return call_at_lower_strike-2*call_at_mid_strike+
           call_at_upper_strike;
}

inline bool nonnegative_equal_strike_butterfly(double low,double mid,double high,
                                               double tolerance=1e-10) {
    require(std::isfinite(tolerance)&&tolerance>=0,
            "Invalid butterfly arbitrage tolerance");
    return butterfly_call_cost(low,mid,high)>=-tolerance;
}
} // namespace kline
