#pragma once
#include "bond_risk_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double risky_bond_price_with_spread(
    const std::vector<double>& times,const std::vector<double>& cashflows,
    double base_yield,double spread){
    require(std::isfinite(base_yield)&&std::isfinite(spread),
            "Invalid additive credit spread");
    return bond_price_from_yield(times,cashflows,base_yield+spread);
}
inline double credit_spread_dv01(
    const std::vector<double>& times,const std::vector<double>& cashflows,
    double base_yield,double spread){
    double up=risky_bond_price_with_spread(times,cashflows,base_yield,spread+1e-4);
    double down=risky_bond_price_with_spread(times,cashflows,base_yield,spread-1e-4);
    return 0.5*(down-up);
}
inline double credit_spread_duration(
    const std::vector<double>& times,const std::vector<double>& cashflows,
    double base_yield,double spread){
    double price=risky_bond_price_with_spread(times,cashflows,base_yield,spread);
    require(price>0,"Credit spread duration needs positive price");
    return credit_spread_dv01(times,cashflows,base_yield,spread)/
           (price*1e-4);
}
} // namespace kline
