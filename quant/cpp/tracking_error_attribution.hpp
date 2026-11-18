#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double ex_ante_tracking_error(
    const Matrix& covariance,const std::vector<double>& active_weights){
    return std::sqrt(portfolio_variance(covariance,active_weights));
}
inline std::vector<double> tracking_error_risk_contributions(
    const Matrix& covariance,const std::vector<double>& active_weights){
    check_covariance(covariance);check_finite(active_weights);
    require(covariance.size()==active_weights.size(),
            "Tracking error asset dimensions");
    auto marginal=covariance_times_weights(covariance,active_weights);
    double total=std::inner_product(active_weights.begin(),active_weights.end(),
                                    marginal.begin(),0.0);
    require(total>0,"Tracking error contribution needs positive variance");
    std::vector<double> contribution(active_weights.size());
    for(int i=0;i<int(contribution.size());i++)
        contribution[i]=active_weights[i]*marginal[i]/std::sqrt(total);
    return contribution;
}
} // namespace kline
