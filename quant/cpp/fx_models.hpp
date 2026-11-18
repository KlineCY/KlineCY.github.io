#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double fx_cross_rate(double quote_per_base,double target_per_quote) {
    require(std::isfinite(quote_per_base)&&quote_per_base>0&&
            std::isfinite(target_per_quote)&&target_per_quote>0,
            "Invalid cross-currency quote");
    return quote_per_base*target_per_quote;
}

inline double fx_inverse(double quote_per_base) {
    require(std::isfinite(quote_per_base)&&quote_per_base>0,
            "FX quote must be strictly positive");
    return 1/quote_per_base;
}

inline double foreign_asset_return_in_base(
    double asset_return_foreign,double base_per_foreign_start,
    double base_per_foreign_end) {
    require(std::isfinite(asset_return_foreign)&&asset_return_foreign>=-1&&
            std::isfinite(base_per_foreign_start)&&base_per_foreign_start>0&&
            std::isfinite(base_per_foreign_end)&&base_per_foreign_end>0,
            "Invalid currency-translated return");
    return (1+asset_return_foreign)*
           base_per_foreign_end/base_per_foreign_start-1;
}

inline double fx_forward_hedge_pnl(double foreign_notional,
                                   double strike_base_per_foreign,
                                   double settlement_base_per_foreign) {
    require(std::isfinite(foreign_notional)&&std::isfinite(strike_base_per_foreign)&&
            strike_base_per_foreign>0&&std::isfinite(settlement_base_per_foreign)&&
            settlement_base_per_foreign>0,"Invalid FX forward settlement");
    return foreign_notional*(strike_base_per_foreign-settlement_base_per_foreign);
}

inline double forward_premium_annualized(
    double spot,double forward,double years) {
    require(std::isfinite(spot)&&spot>0&&std::isfinite(forward)&&forward>0&&
            std::isfinite(years)&&years>0,"Invalid forward premium inputs");
    return std::log(forward/spot)/years;
}
} // namespace kline
