#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct GreekHedge{double underlying=0,option=0;};
inline GreekHedge delta_gamma_hedge(
    double portfolio_delta,double portfolio_gamma,
    double hedge_option_delta,double hedge_option_gamma){
    require(std::isfinite(portfolio_delta)&&std::isfinite(portfolio_gamma)&&
            std::isfinite(hedge_option_delta)&&
            std::isfinite(hedge_option_gamma)&&
            std::fabs(hedge_option_gamma)>1e-12,
            "Invalid delta gamma neutralization sensitivities");
    double option=-portfolio_gamma/hedge_option_gamma;
    double stock=-portfolio_delta-option*hedge_option_delta;
    return {stock,option};
}
inline double second_order_greeks_pnl(
    double delta,double gamma,double vega,
    double spot_change,double volatility_change){
    require(std::isfinite(delta)&&std::isfinite(gamma)&&
            std::isfinite(vega)&&std::isfinite(spot_change)&&
            std::isfinite(volatility_change),"Nonfinite Greek scenario");
    return delta*spot_change+0.5*gamma*spot_change*spot_change+
           vega*volatility_change;
}
} // namespace kline
