#pragma once
#include "ols_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct Autoregression{
    std::vector<double> coefficients;
    double residual_variance=0;
    int lag=0;
};
inline Autoregression fit_autoregression(const std::vector<double>& x,int lag){
    check_finite(x);
    require(lag>=1&&int(x.size())>=2*lag+3,
            "Insufficient data to estimate AR lag");
    Matrix feature;std::vector<double> target;
    for(int t=lag;t<int(x.size());t++){
        std::vector<double> row;
        for(int j=1;j<=lag;j++)row.push_back(x[t-j]);
        feature.push_back(row);target.push_back(x[t]);
    }
    auto fitted=ordinary_least_squares(feature,target);
    return {fitted.coefficients,fitted.mse,lag};
}
inline double autoregressive_forecast(const Autoregression& model,
                                      const std::vector<double>& x){
    check_finite(x);
    require(model.lag>0&&int(x.size())>=model.lag&&
            int(model.coefficients.size())==model.lag+1,
            "Invalid autoregression forecast");
    double result=model.coefficients[0];
    for(int j=1;j<=model.lag;j++)
        result+=model.coefficients[j]*x[x.size()-j];
    return result;
}
inline std::vector<double> difference_series(const std::vector<double>& x,
                                              int order){
    check_finite(x);
    require(order>=1&&order<int(x.size()),"Invalid differencing order");
    std::vector<double> out=x;
    for(int j=0;j<order;j++){
        std::vector<double> next;
        for(int t=1;t<int(out.size());t++)next.push_back(out[t]-out[t-1]);
        out=next;
    }
    return out;
}
inline double ar_long_run_mean(const Autoregression& model){
    require(model.lag>0,"Invalid AR model");
    double denominator=1;
    for(int j=1;j<=model.lag;j++)denominator-=model.coefficients[j];
    require(std::fabs(denominator)>1e-10,
            "AR model has no finite long-run intercept mean");
    return model.coefficients[0]/denominator;
}
} // namespace kline
