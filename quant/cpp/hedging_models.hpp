#pragma once
#include "derivatives.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double delta_gamma_vega_pnl(double delta,double gamma,double vega,
                                    double spot_change,double vol_change) {
    require(std::isfinite(delta)&&std::isfinite(gamma)&&std::isfinite(vega)&&
            std::isfinite(spot_change)&&std::isfinite(vol_change),
            "Invalid option hedge sensitivities");
    return delta*spot_change+0.5*gamma*spot_change*spot_change+
           vega*vol_change;
}

inline double hedge_contract_count(double cash_exposure,
                                   double hedge_ratio,double futures_price,
                                   double contract_multiplier) {
    require(std::isfinite(cash_exposure)&&
            std::isfinite(hedge_ratio)&&std::isfinite(futures_price)&&
            futures_price>0&&std::isfinite(contract_multiplier)&&
            contract_multiplier>0,"Invalid futures hedge inputs");
    return -cash_exposure*hedge_ratio/(futures_price*contract_multiplier);
}

inline double futures_variation_margin(double contracts,double multiplier,
                                       double settle_previous,double settle_today) {
    require(std::isfinite(contracts)&&std::isfinite(multiplier)&&multiplier>0&&
            std::isfinite(settle_previous)&&std::isfinite(settle_today),
            "Invalid futures mark-to-market inputs");
    return contracts*multiplier*(settle_today-settle_previous);
}

inline double option_delta_hedged_pnl(double option_before,double option_after,
                                      double hedge_shares,double stock_before,
                                      double stock_after,double funding_cost) {
    require(std::isfinite(option_before)&&std::isfinite(option_after)&&
            std::isfinite(hedge_shares)&&std::isfinite(stock_before)&&
            std::isfinite(stock_after)&&std::isfinite(funding_cost)&&
            funding_cost>=0,"Invalid delta hedge PnL inputs");
    return option_after-option_before+
           hedge_shares*(stock_after-stock_before)-funding_cost;
}

inline double put_call_synthetic_forward(double call,double put,double strike,
                                          double discount_factor) {
    require(std::isfinite(call)&&std::isfinite(put)&&
            std::isfinite(strike)&&strike>0&&
            std::isfinite(discount_factor)&&discount_factor>0,
            "Invalid put-call synthetic forward inputs");
    return strike+(call-put)/discount_factor;
}
} // namespace kline
