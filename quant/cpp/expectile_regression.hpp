#pragma once
#include "weighted_qr_regression.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ExpectileFit{std::vector<double> coefficients;};
inline ExpectileFit fit_linear_expectile(
    const Matrix& x,const std::vector<double>& y,double expectile,
    int iterations=100){
    check_feature_table(x,y);
    require(std::isfinite(expectile)&&expectile>0&&expectile<1&&
            iterations>0,"Invalid expectile regression parameters");
    std::vector<double> weights(y.size(),1);
    auto fit=weighted_qr_intercept(x,y,weights);
    for(int it=0;it<iterations;it++){
        for(int t=0;t<int(y.size());t++){
            double prediction=fit.coefficients[0];
            for(int j=0;j<int(x[t].size());j++)
                prediction+=fit.coefficients[j+1]*x[t][j];
            weights[t]=y[t]>=prediction?expectile:1-expectile;
        }
        auto next=weighted_qr_intercept(x,y,weights);
        double change=0;
        for(int j=0;j<int(next.coefficients.size());j++)
            change=std::max(change,std::fabs(
                next.coefficients[j]-fit.coefficients[j]));
        fit=next;
        if(change<1e-9)break;
    }
    return {fit.coefficients};
}
} // namespace kline
