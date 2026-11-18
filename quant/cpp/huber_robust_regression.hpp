#pragma once
#include "weighted_qr_regression.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct HuberFit{std::vector<double> coefficients;int iterations=0;};
inline HuberFit huber_regression(const Matrix& x,const std::vector<double>& y,
               double threshold,int iterations=100){
    check_feature_table(x,y);
    require(std::isfinite(threshold)&&threshold>0&&iterations>0,
            "Invalid Huber regression parameters");
    std::vector<double> weights(y.size(),1);
    auto fit=weighted_qr_intercept(x,y,weights);
    for(int step=0;step<iterations;step++){
        for(int t=0;t<int(y.size());t++){
            double predicted=fit.coefficients[0];
            for(int j=0;j<int(x[t].size());j++)
                predicted+=fit.coefficients[j+1]*x[t][j];
            double residual=std::fabs(y[t]-predicted);
            weights[t]=residual<=threshold?1:threshold/residual;
        }
        auto next=weighted_qr_intercept(x,y,weights);
        double change=0;
        for(int j=0;j<int(fit.coefficients.size());j++)
            change=std::max(change,std::fabs(
                next.coefficients[j]-fit.coefficients[j]));
        fit=next;
        if(change<1e-9)return {fit.coefficients,step+1};
    }
    return {fit.coefficients,iterations};
}
} // namespace kline
