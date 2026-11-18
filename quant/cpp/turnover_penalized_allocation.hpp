#pragma once
#include "allocation.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double turnover_penalized_objective(const std::vector<double>& w,
         const std::vector<double>& mu,const Matrix& cov,
         const std::vector<double>& previous,double gamma,double fee){
    check_finite(w);check_finite(mu);check_finite(previous);
    require(w.size()==mu.size()&&w.size()==previous.size()&&
            std::isfinite(gamma)&&gamma>=0&&
            std::isfinite(fee)&&fee>=0,"Turnover allocation dimensions");
    double utility=0;
    for(int i=0;i<int(w.size());i++)
        utility+=mu[i]*w[i]-fee*std::fabs(w[i]-previous[i]);
    return -utility+gamma*portfolio_variance(cov,w)*0.5;
}
inline std::vector<double> projected_turnover_allocation(
        const std::vector<double>& expected,const Matrix& covariance,
        const std::vector<double>& previous,double gamma,double fee,
        int iterations=3000,double step=0.1){
    require_positive_semidefinite(covariance);
    check_finite(expected);check_finite(previous);
    int n=int(expected.size());
    require(n>0&&int(previous.size())==n&&int(covariance.size())==n&&
            gamma>=0&&std::isfinite(gamma)&&fee>=0&&std::isfinite(fee)&&
            iterations>0&&step>0&&std::isfinite(step),
            "Turnover allocation invalid");
    std::vector<double> w=project_simplex(previous),average(n,0);
    for(int t=1;t<=iterations;t++){
        auto marginal=covariance_times_weights(covariance,w);
        std::vector<double> next(n);
        for(int i=0;i<n;i++){
            double sign=w[i]>previous[i]+1e-10?1:
                        w[i]<previous[i]-1e-10?-1:0;
            double gradient=gamma*marginal[i]-expected[i]+fee*sign;
            next[i]=w[i]-step/std::sqrt(t)*gradient;
        }
        w=project_simplex(next);
        for(int i=0;i<n;i++)average[i]+=w[i]/iterations;
    }
    return project_simplex(average);
}
} // namespace kline
