#pragma once
#include "allocation.hpp"
#include "matrix_algorithms.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline Matrix factor_structured_covariance(const Matrix& exposures,
                 const Matrix& factor_covariance,
                 const std::vector<double>& specific_variances){
    require(!exposures.empty()&&!exposures[0].empty(),
            "Factor model needs assets and factors");
    require_positive_semidefinite(factor_covariance);
    check_finite(specific_variances);
    int n=int(exposures.size()),k=int(exposures[0].size());
    require(int(factor_covariance.size())==k&&
            int(specific_variances.size())==n,"Factor risk dimensions");
    for(int i=0;i<n;i++){
        check_finite(exposures[i]);
        require(int(exposures[i].size())==k&&specific_variances[i]>=0,
                "Factor exposure or idiosyncratic variance");
    }
    auto result=matrix_multiply(
        matrix_multiply(exposures,factor_covariance),
        matrix_transpose(exposures));
    for(int i=0;i<n;i++)result[i][i]+=specific_variances[i];
    return result;
}
inline std::vector<double> portfolio_factor_exposure(
    const Matrix& asset_by_factor,const std::vector<double>& weights){
    check_finite(weights);
    require(asset_by_factor.size()==weights.size()&&
            !asset_by_factor.empty(),"Portfolio factor dimensions");
    int k=int(asset_by_factor[0].size());
    std::vector<double> exposure(k,0);
    for(int i=0;i<int(weights.size());i++){
        check_finite(asset_by_factor[i]);
        require(int(asset_by_factor[i].size())==k,"Factor ragged design");
        for(int j=0;j<k;j++)exposure[j]+=weights[i]*asset_by_factor[i][j];
    }
    return exposure;
}
} // namespace kline
