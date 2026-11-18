#pragma once
#include "autoregressive_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ArimaDifferenceModel{Autoregression ar;int difference_order=1;double last_level=0;};
inline ArimaDifferenceModel fit_arima_p1_0(
    const std::vector<double>& levels,int autoregression_lags){
    check_finite(levels);require(levels.size()>=12,"ARIMA needs 12 observations");
    auto changes=difference_series(levels,1);
    require(autoregression_lags>=1&&
            int(changes.size())>=2*autoregression_lags+3,
            "ARIMA lag needs sufficient sample length");
    if(variance(changes,false)<1e-20){
        Autoregression model;
        model.lag=autoregression_lags;
        model.coefficients.assign(autoregression_lags+1,0.0);
        model.coefficients[0]=mean(changes);
        return {model,1,levels.back()};
    }
    return {fit_autoregression(changes,autoregression_lags),1,levels.back()};
}
inline double forecast_arima_one_step(
    const ArimaDifferenceModel& model,const std::vector<double>& levels){
    check_finite(levels);
    require(model.difference_order==1&&levels.size()>=2,
            "ARIMA currently supports one difference");
    auto changes=difference_series(levels,1);
    return levels.back()+autoregressive_forecast(model.ar,changes);
}
} // namespace kline
