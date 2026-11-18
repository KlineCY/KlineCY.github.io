#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct CapacityEconomics{double gross_profit=0,impact_cost=0,net_profit=0;};
inline CapacityEconomics square_root_capacity_profit(
    double deployed_capital,double gross_alpha_fraction,
    double annual_turnover,double market_impact_coefficient){
    require(std::isfinite(deployed_capital)&&deployed_capital>=0&&
            std::isfinite(gross_alpha_fraction)&&
            std::isfinite(annual_turnover)&&annual_turnover>=0&&
            std::isfinite(market_impact_coefficient)&&
            market_impact_coefficient>=0,"Invalid strategy capacity inputs");
    double gross=deployed_capital*gross_alpha_fraction;
    double impact=market_impact_coefficient*annual_turnover*
                  std::pow(deployed_capital,1.5);
    return {gross,impact,gross-impact};
}
inline double square_root_capacity_optimal_capital(
    double alpha_fraction,double turnover,double impact_coefficient){
    require(std::isfinite(alpha_fraction)&&alpha_fraction>=0&&
            std::isfinite(turnover)&&turnover>0&&
            std::isfinite(impact_coefficient)&&impact_coefficient>0,
            "Capacity optimum needs positive cost and turnover");
    double ratio=2*alpha_fraction/(3*turnover*impact_coefficient);
    return ratio*ratio;
}
} // namespace kline
