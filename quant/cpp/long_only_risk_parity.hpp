#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <numeric>
namespace kline {
inline std::vector<double> positive_covariance_risk_parity(
    const Matrix& covariance,int iterations=1000,double tolerance=1e-9){
    check_covariance(covariance);
    int n=int(covariance.size());
    require(iterations>=1&&tolerance>0&&
            std::isfinite(tolerance),"Invalid risk parity iterations");
    for(int i=0;i<n;i++)for(int j=0;j<n;j++){
        require(covariance[i][j]>=0,
                "Multiplicative risk parity requires nonnegative covariance");
        if(i==j)require(covariance[i][i]>0,
                        "Risk parity needs positive asset variance");
    }
    std::vector<double> w(n,1.0/n);
    for(int step=0;step<iterations;step++){
        auto marginal=covariance_times_weights(covariance,w);
        double total=0;
        for(int i=0;i<n;i++)total+=w[i]*marginal[i];
        double target=total/n;
        std::vector<double> next=w;
        for(int i=0;i<n;i++)
            next[i]*=std::sqrt(target/(w[i]*marginal[i]));
        double normalizer=std::accumulate(next.begin(),next.end(),0.0);
        for(double& v:next)v/=normalizer;
        double change=0;
        for(int i=0;i<n;i++)
            change=std::max(change,std::fabs(next[i]-w[i]));
        w=next;
        if(change<tolerance)break;
    }
    return w;
}
} // namespace kline
