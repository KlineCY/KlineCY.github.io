#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double net_present_value(double initial_investment,
    const std::vector<double>& future_cashflows,double discount){
    check_finite(future_cashflows);
    require(std::isfinite(initial_investment)&&
            std::isfinite(discount)&&discount>-1,
            "Invalid NPV inputs");
    double value=-initial_investment;
    for(int t=0;t<int(future_cashflows.size());t++)
        value+=future_cashflows[t]/std::pow(1+discount,t+1);
    return value;
}
inline double weighted_average_cost_of_capital(double equity_weight,
    double cost_equity,double pretax_cost_debt,double tax_rate){
    require(std::isfinite(equity_weight)&&equity_weight>=0&&equity_weight<=1&&
            std::isfinite(cost_equity)&&std::isfinite(pretax_cost_debt)&&
            std::isfinite(tax_rate)&&tax_rate>=0&&tax_rate<=1,
            "Invalid WACC values");
    return equity_weight*cost_equity+
           (1-equity_weight)*pretax_cost_debt*(1-tax_rate);
}
inline double perpetuity_value(double next_cashflow,double growth,
                                double discount){
    require(std::isfinite(next_cashflow)&&std::isfinite(growth)&&
            std::isfinite(discount)&&discount>growth,
            "Perpetuity growth must be below discount rate");
    return next_cashflow/(discount-growth);
}
} // namespace kline
