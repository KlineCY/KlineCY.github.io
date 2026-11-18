#pragma once
#include "portfolio.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline double correlated_stress_sigma(const Matrix& covariance,
                                       const std::vector<double>& weights) {
    return std::sqrt(portfolio_variance(covariance,weights));
}

inline std::vector<double> scenario_portfolio_pnl(
    const std::vector<double>& weights,const Matrix& scenarios) {
    check_finite(weights);
    require(!scenarios.empty(),"Stress scenarios missing");
    std::vector<double> pnl;
    for(const auto& row:scenarios){
        check_finite(row);
        require(row.size()==weights.size(),"Stress asset count differs");
        pnl.push_back(std::inner_product(weights.begin(),weights.end(),
                                         row.begin(),0.0));
    }
    return pnl;
}

inline double worst_scenario_return(const std::vector<double>& weights,
                                    const Matrix& scenarios) {
    auto outcomes=scenario_portfolio_pnl(weights,scenarios);
    return *std::min_element(outcomes.begin(),outcomes.end());
}

inline double volatility_target_gross(double volatility,double target,
                                      double previous_leverage,
                                      double max_change,double cap) {
    require(std::isfinite(volatility)&&volatility>0&&std::isfinite(target)&&
            target>=0&&std::isfinite(previous_leverage)&&previous_leverage>=0&&
            std::isfinite(max_change)&&max_change>=0&&std::isfinite(cap)&&
            cap>=0,"Invalid volatility targeting parameters");
    return std::max(0.0,std::min({cap,target/volatility,
                                  previous_leverage+max_change}));
}

inline std::vector<double> normalize_gross(
    const std::vector<double>& signal,double gross_limit) {
    check_finite(signal);
    require(std::isfinite(gross_limit)&&gross_limit>=0,
            "Gross exposure must be nonnegative");
    double norm=0;
    for(double v:signal)norm+=std::fabs(v);
    if(norm==0)return std::vector<double>(signal.size(),0);
    std::vector<double> result=signal;
    for(double& value:result)value*=gross_limit/norm;
    return result;
}

inline double expected_margin_buffer(double equity,double gross_exposure,
                                      double maintenance_fraction) {
    require(std::isfinite(equity)&&equity>=0&&
            std::isfinite(gross_exposure)&&gross_exposure>=0&&
            std::isfinite(maintenance_fraction)&&maintenance_fraction>=0&&
            maintenance_fraction<=1,"Invalid maintenance margin inputs");
    return equity*(1-gross_exposure*maintenance_fraction);
}
} // namespace kline
