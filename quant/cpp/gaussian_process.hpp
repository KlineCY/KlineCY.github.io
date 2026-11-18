#pragma once
#include "kernel_ridge.hpp"
#include <cmath>
namespace kline {
struct GpPrediction{double mean=0,latent_variance=0;};
inline GpPrediction gaussian_process_predict(const Matrix& observations,
                   const std::vector<double>& targets,
                   const std::vector<double>& query,
                   double lengthscale,double noise_variance){
    check_feature_table(observations,targets);
    check_finite(query);
    require(query.size()==observations[0].size()&&
            std::isfinite(noise_variance)&&noise_variance>0,
            "Invalid Gaussian process noise or query");
    int n=int(observations.size());
    Matrix covariance(n,std::vector<double>(n));
    std::vector<double> cross(n);
    for(int i=0;i<n;i++){
        for(int j=0;j<=i;j++)
            covariance[i][j]=covariance[j][i]=squared_exponential_kernel(
                observations[i],observations[j],lengthscale);
        cross[i]=squared_exponential_kernel(observations[i],query,lengthscale);
        covariance[i][i]+=noise_variance;
    }
    auto weights=solve_cholesky(covariance,targets);
    auto solve=solve_cholesky(covariance,cross);
    double mu=0,var=1;
    for(int i=0;i<n;i++){mu+=cross[i]*weights[i];var-=cross[i]*solve[i];}
    require(var>=-1e-8&&std::isfinite(mu),"Gaussian process covariance invalid");
    return {mu,std::max(0.0,var)};
}
} // namespace kline
