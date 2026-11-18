#pragma once
#include "allocation.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<double> risk_budget_weights(const Matrix& covariance,
       const std::vector<double>& budgets,int iterations=6000,
       double learning_rate=0.15){
    require_positive_semidefinite(covariance);check_finite(budgets);
    int n=int(covariance.size());
    require(int(budgets.size())==n&&iterations>0&&learning_rate>0&&
            std::isfinite(learning_rate),"Risk budget input dimensions");
    double total=std::accumulate(budgets.begin(),budgets.end(),0.0);
    require(total>0,"Risk budget total must be positive");
    for(int i=0;i<n;i++)
        require(budgets[i]>0&&covariance[i][i]>0,
                "Risk budgets and variances must be positive");
    std::vector<double> w(n);
    for(int i=0;i<n;i++)w[i]=budgets[i]/total;
    for(int step=0;step<iterations;step++){
        auto marginal=covariance_times_weights(covariance,w);
        double variance=std::inner_product(w.begin(),w.end(),
                                           marginal.begin(),0.0);
        require(variance>0,"Risk budget variance vanishes");
        double max_error=0;std::vector<double> next(n);
        for(int i=0;i<n;i++){
            double share=w[i]*marginal[i]/variance;
            max_error=std::max(max_error,std::fabs(share-budgets[i]/total));
            double scaled=std::max(-30.0,std::min(30.0,
                       learning_rate*(budgets[i]/total-share)));
            next[i]=w[i]*std::exp(scaled);
        }
        double sum=std::accumulate(next.begin(),next.end(),0.0);
        for(int i=0;i<n;i++)w[i]=next[i]/sum;
        if(max_error<1e-8)return w;
    }
    throw std::runtime_error("Risk budget allocation did not converge");
}
} // namespace kline
