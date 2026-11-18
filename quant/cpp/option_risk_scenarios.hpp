#pragma once
#include "derivatives.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct OptionShock{
    double stock_change=0,vol_change=0,rate_change=0;
};
inline double reprice_option_shock(double spot,double strike,double rate,
                                    double dividend,double vol,double years,
                                    const OptionShock& shock,bool call=true){
    require(std::isfinite(shock.stock_change)&&
            std::isfinite(shock.vol_change)&&std::isfinite(shock.rate_change),
            "Invalid option scenario shock");
    double next_spot=spot+shock.stock_change;
    double next_vol=vol+shock.vol_change;
    require(next_spot>0&&next_vol>0,"Shock invalidates spot or volatility");
    auto before=black_scholes(spot,strike,rate,dividend,vol,years,call);
    auto after=black_scholes(next_spot,strike,rate+shock.rate_change,
                             dividend,next_vol,years,call);
    return after.value-before.value;
}
inline double gamma_scalping_approx(double gamma,double volatility,
                                    double realized_quadratic_variation,
                                    double horizon,double spot){
    require(std::isfinite(gamma)&&std::isfinite(volatility)&&volatility>=0&&
            std::isfinite(realized_quadratic_variation)&&
            realized_quadratic_variation>=0&&std::isfinite(horizon)&&
            horizon>=0&&std::isfinite(spot)&&spot>0,
            "Gamma scalping approximation");
    return 0.5*gamma*spot*spot*
           (realized_quadratic_variation-volatility*volatility*horizon);
}
} // namespace kline
