#pragma once
#include "factor_risk_covariance.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> factor_scenario_asset_returns(
    const Matrix& exposures,const std::vector<double>& shock,
    const std::vector<double>& specific_shock){
    check_finite(shock);check_finite(specific_shock);
    require(exposures.size()==specific_shock.size()&&!exposures.empty(),
            "Factor stress scenario dimensions");
    std::vector<double> returns(exposures.size());
    for(int i=0;i<int(exposures.size());i++){
        check_finite(exposures[i]);
        require(exposures[i].size()==shock.size(),"Factor shock dimensions");
        returns[i]=specific_shock[i];
        for(int j=0;j<int(shock.size());j++)
            returns[i]+=exposures[i][j]*shock[j];
    }
    return returns;
}
inline double portfolio_factor_scenario_loss(
    const std::vector<double>& weights,const Matrix& exposures,
    const std::vector<double>& factor_shock,
    const std::vector<double>& idiosyncratic){
    auto returns=factor_scenario_asset_returns(exposures,factor_shock,
                                               idiosyncratic);
    check_finite(weights);
    require(weights.size()==returns.size(),"Scenario portfolio shape");
    double pnl=0;
    for(int i=0;i<int(weights.size());i++)pnl+=weights[i]*returns[i];
    return -pnl;
}
} // namespace kline
