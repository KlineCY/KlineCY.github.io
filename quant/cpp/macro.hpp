#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {

inline double continuous_discount_factor(double zero_rate, double years) {
    require(std::isfinite(zero_rate) && years >= 0 && std::isfinite(years),
            "Invalid continuously compounded zero rate or maturity");
    return std::exp(-zero_rate * years);
}

inline double continuous_forward_rate(double zero_rate_short, double short_years,
                                       double zero_rate_long, double long_years) {
    require(short_years >= 0 && long_years > short_years &&
            std::isfinite(zero_rate_short) && std::isfinite(zero_rate_long),
            "Forward tenor must be increasing");
    return (zero_rate_long * long_years - zero_rate_short * short_years) /
           (long_years - short_years);
}

inline double term_spread_bps(double long_yield, double short_yield) {
    require(std::isfinite(long_yield) && std::isfinite(short_yield),
            "Yields must be finite");
    return (long_yield - short_yield) * 10000;
}

inline double breakeven_inflation(double nominal_zero_yield, double real_zero_yield) {
    require(std::isfinite(nominal_zero_yield) && std::isfinite(real_zero_yield),
            "Zero rates must be finite");
    // This difference can also include liquidity and inflation risk premia.
    return nominal_zero_yield - real_zero_yield;
}

inline double approximate_parallel_shift_pnl(double bond_market_value,
                                              double modified_duration,
                                              double convexity,
                                              double yield_change) {
    require(bond_market_value >= 0 && modified_duration >= 0 && convexity >= 0 &&
            std::isfinite(bond_market_value) && std::isfinite(yield_change),
            "Invalid rate-shock parameters");
    double fractional_change =
        -modified_duration * yield_change +
        0.5 * convexity * yield_change * yield_change;
    return bond_market_value * fractional_change;
}

inline double duration_hedge_notional(double asset_market_value,
                                       double asset_duration, double hedge_duration) {
    require(asset_market_value >= 0 && asset_duration >= 0 && hedge_duration > 0,
            "Invalid asset or hedge duration");
    // Sign convention: negative means short the hedging instrument.
    return -asset_market_value * asset_duration / hedge_duration;
}

inline double fx_forward_quote_per_base(double spot_quote_per_base,
                                         double base_annual_rate,
                                         double quote_annual_rate,
                                         double years) {
    require(spot_quote_per_base > 0 && years >= 0 &&
            std::isfinite(base_annual_rate) && std::isfinite(quote_annual_rate),
            "Invalid FX forward inputs");
    require(1 + base_annual_rate * years > 0 &&
            1 + quote_annual_rate * years > 0, "Money-market factors must be positive");
    // Spot is units of quote currency for one unit of base currency.
    // Covered interest parity: forward = spot*(1+r_quote*T)/(1+r_base*T).
    return spot_quote_per_base * (1 + quote_annual_rate * years) /
           (1 + base_annual_rate * years);
}

inline double futures_roll_yield(double expiring_contract_price,
                                  double deferred_contract_price) {
    require(expiring_contract_price > 0 && deferred_contract_price > 0 &&
            std::isfinite(expiring_contract_price) && std::isfinite(deferred_contract_price),
            "Futures prices must be positive for this proportional roll measure");
    // A simplified static roll measure; realized futures returns also depend on
    // subsequent changes in the contracts' prices.
    return expiring_contract_price / deferred_contract_price - 1;
}

inline double excess_bond_return_approx(double carry_yield,
                                         double modified_duration,
                                         double yield_change, double convexity) {
    require(modified_duration >= 0 && convexity >= 0 &&
            std::isfinite(carry_yield) && std::isfinite(yield_change),
            "Invalid approximate bond return inputs");
    return carry_yield - modified_duration * yield_change +
           0.5 * convexity * yield_change * yield_change;
}

}  // namespace kline
