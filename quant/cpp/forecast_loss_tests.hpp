#pragma once
#include "hac_mean_inference.hpp"
#include <cmath>
namespace kline {
struct LossComparison{double average_difference=0,standard_error=0,standardized=0;};
inline LossComparison squared_forecast_loss_comparison(
    const std::vector<double>& truth,
    const std::vector<double>& forecast_one,
    const std::vector<double>& forecast_two,int hac_lags){
    check_finite(truth);check_finite(forecast_one);check_finite(forecast_two);
    require(truth.size()==forecast_one.size()&&
            truth.size()==forecast_two.size(),
            "Forecast loss lengths must match");
    std::vector<double> differences;
    for(int i=0;i<int(truth.size());i++){
        double a=std::pow(truth[i]-forecast_one[i],2);
        double b=std::pow(truth[i]-forecast_two[i],2);
        differences.push_back(a-b);
    }
    auto report=bartlett_hac_mean(differences,hac_lags);
    return {report.estimate,report.standard_error,report.t_stat};
}
} // namespace kline
