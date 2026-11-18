#pragma once
#include "cholesky_numerics.hpp"
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct BayesianLinearFit{
    std::vector<double> posterior_mean;
    Matrix posterior_covariance;
    double observation_variance=1;
};
inline BayesianLinearFit bayesian_linear_posterior(
  const Matrix& features,const std::vector<double>& targets,
  double observation_variance,double prior_variance){
    check_feature_table(features,targets);
    require(std::isfinite(observation_variance)&&observation_variance>0&&
            std::isfinite(prior_variance)&&prior_variance>0,
            "Invalid Bayesian linear observation or prior variance");
    int n=int(features.size()),p=int(features[0].size())+1;
    Matrix precision(p,std::vector<double>(p,0));
    std::vector<double> rhs(p,0);
    for(int j=0;j<p;j++)precision[j][j]=1/prior_variance;
    for(int i=0;i<n;i++){
        std::vector<double> x={1};
        x.insert(x.end(),features[i].begin(),features[i].end());
        for(int j=0;j<p;j++){
            rhs[j]+=x[j]*targets[i]/observation_variance;
            for(int k=0;k<p;k++)
                precision[j][k]+=x[j]*x[k]/observation_variance;
        }
    }
    auto mu=solve_cholesky(precision,rhs);
    Matrix covariance(p,std::vector<double>(p));
    for(int j=0;j<p;j++){
        std::vector<double> unit(p,0);unit[j]=1;
        auto column=solve_cholesky(precision,unit);
        for(int i=0;i<p;i++)covariance[i][j]=column[i];
    }
    return {mu,covariance,observation_variance};
}
inline std::pair<double,double> bayesian_linear_prediction(
   const BayesianLinearFit& model,const std::vector<double>& row){
    check_finite(row);
    require(row.size()+1==model.posterior_mean.size(),
            "Bayesian prediction dimensions");
    std::vector<double> x={1};x.insert(x.end(),row.begin(),row.end());
    double predicted=0,variance=model.observation_variance;
    for(int i=0;i<int(x.size());i++){
        predicted+=x[i]*model.posterior_mean[i];
        for(int j=0;j<int(x.size());j++)
            variance+=x[i]*model.posterior_covariance[i][j]*x[j];
    }
    return {predicted,variance};
}
} // namespace kline
