#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct QuantileLinearFit{std::vector<double> coefficients;double pinball=0;};
inline QuantileLinearFit fit_quantile_linear(
    const Matrix& x,const std::vector<double>& y,double quantile,
    double l2=0.001,int epochs=2000,double step=0.08){
    check_feature_table(x,y);
    require(std::isfinite(quantile)&&quantile>0&&quantile<1&&
            std::isfinite(l2)&&l2>=0&&
            epochs>0&&std::isfinite(step)&&step>0,
            "Invalid quantile regression parameters");
    int n=int(x.size()),p=int(x[0].size())+1;
    std::vector<double> beta(p,0);
    beta[0]=mean(y);
    for(int iteration=1;iteration<=epochs;iteration++){
        std::vector<double> gradient(p,0);
        for(int i=0;i<n;i++){
            double prediction=beta[0];
            for(int j=1;j<p;j++)prediction+=beta[j]*x[i][j-1];
            double residual=y[i]-prediction;
            double derivative=residual>=0?-quantile:1-quantile;
            gradient[0]+=derivative/n;
            for(int j=1;j<p;j++)
                gradient[j]+=derivative*x[i][j-1]/n;
        }
        double eta=step/std::sqrt(iteration);
        for(int j=0;j<p;j++){
            if(j>0)gradient[j]+=l2*beta[j];
            beta[j]-=eta*gradient[j];
        }
    }
    double loss=0;
    for(int i=0;i<n;i++){
        double prediction=beta[0];
        for(int j=1;j<p;j++)prediction+=beta[j]*x[i][j-1];
        double residual=y[i]-prediction;
        loss+=(residual>=0?quantile*residual:(quantile-1)*residual)/n;
    }
    return {beta,loss};
}
} // namespace kline
