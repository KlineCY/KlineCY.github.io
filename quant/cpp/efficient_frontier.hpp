#pragma once
#include "matrix_algorithms.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<double> unconstrained_min_variance(
    const Matrix& covariance){
    check_covariance(covariance);
    int n=int(covariance.size());
    auto raw=solve_linear_system(covariance,std::vector<double>(n,1));
    double sum=std::accumulate(raw.begin(),raw.end(),0.0);
    require(std::isfinite(sum)&&sum>0,"GMV normalization invalid");
    for(double& v:raw)v/=sum;
    return raw;
}
inline std::vector<double> mean_variance_target_return(
    const Matrix& covariance,const std::vector<double>& expected,
    double target){
    check_covariance(covariance);check_finite(expected);
    require(covariance.size()==expected.size()&&std::isfinite(target)&&
            expected.size()>=2,"Invalid efficient frontier inputs");
    int n=int(expected.size());
    auto ones=solve_linear_system(covariance,std::vector<double>(n,1));
    auto mu=solve_linear_system(covariance,expected);
    double A=std::accumulate(ones.begin(),ones.end(),0.0),B=0,C=0;
    for(int i=0;i<n;i++){B+=expected[i]*ones[i];C+=expected[i]*mu[i];}
    double det=A*C-B*B;
    require(det>1e-12,"Efficient frontier degenerate expected returns");
    double lambda=(C-B*target)/det;
    double gamma=(A*target-B)/det;
    std::vector<double> w(n);
    for(int i=0;i<n;i++)w[i]=lambda*ones[i]+gamma*mu[i];
    return w;
}
inline double annualized_portfolio_volatility(const Matrix& covariance,
    const std::vector<double>& weights,int periods=252){
    require(periods>0,"Invalid portfolio annualization");
    return std::sqrt(portfolio_variance(covariance,weights)*periods);
}
} // namespace kline
