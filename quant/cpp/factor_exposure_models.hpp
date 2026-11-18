#pragma once
#include "ols_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> rolling_factor_beta(
    const std::vector<double>& asset_returns,
    const std::vector<double>& factor_returns,int window){
    check_finite(asset_returns);check_finite(factor_returns);
    require(asset_returns.size()==factor_returns.size()&&
            window>=3&&window<=int(asset_returns.size()),
            "Rolling beta window invalid");
    std::vector<double> out;
    for(int t=window;t<=int(asset_returns.size());t++){
        std::vector<double> a(asset_returns.begin()+t-window,
                              asset_returns.begin()+t);
        std::vector<double> f(factor_returns.begin()+t-window,
                              factor_returns.begin()+t);
        double denominator=variance(f,false);
        require(denominator>0,"Constant rolling factor");
        out.push_back(covariance(a,f,false)/denominator);
    }
    return out;
}
inline double factor_neutral_portfolio_beta(
    const std::vector<double>& weights,const std::vector<double>& betas){
    check_finite(weights);check_finite(betas);
    require(weights.size()==betas.size(),"Factor beta dimensions");
    return std::inner_product(weights.begin(),weights.end(),
                              betas.begin(),0.0);
}
inline std::vector<double> hedge_factor_exposure(
    const std::vector<double>& weights,
    const std::vector<double>& betas,int hedging_asset){
    require(weights.size()==betas.size()&&
            hedging_asset>=0&&hedging_asset<int(weights.size()),
            "Invalid beta hedging asset");
    check_finite(weights);check_finite(betas);
    require(std::fabs(betas[hedging_asset])>1e-12,
            "Hedging asset beta cannot be zero");
    std::vector<double> result=weights;
    result[hedging_asset]-=
        factor_neutral_portfolio_beta(weights,betas)/betas[hedging_asset];
    return result;
}
} // namespace kline
