#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct PredictionMetrics{double mae=0,rmse=0,mean_bias=0;};
inline PredictionMetrics out_of_sample_regression_metrics(
    const std::vector<double>& actual,const std::vector<double>& forecast){
    check_finite(actual);check_finite(forecast);
    require(actual.size()==forecast.size(),
            "Regression forecasts and actuals must align");
    double abs_sum=0,square=0,bias=0;
    for(int i=0;i<int(actual.size());i++){
        double error=forecast[i]-actual[i];
        abs_sum+=std::fabs(error);square+=error*error;bias+=error;
    }
    double n=double(actual.size());
    return {abs_sum/n,std::sqrt(square/n),bias/n};
}
} // namespace kline
