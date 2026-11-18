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
