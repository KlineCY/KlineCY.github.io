#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> inverse_error_ensemble_weights(
    const Matrix& historical_forecasts,
    const std::vector<double>& historical_realized,double floor=1e-6){
    check_finite(historical_realized);
    require(historical_forecasts.size()==historical_realized.size()&&
            !historical_forecasts.empty()&&
            std::isfinite(floor)&&floor>0,
            "Invalid ensemble training data");
    int models=int(historical_forecasts[0].size());
    require(models>0,"Ensemble needs models");
    std::vector<double> mse(models,0);
    for(int t=0;t<int(historical_realized.size());t++){
        check_finite(historical_forecasts[t]);
        require(int(historical_forecasts[t].size())==models,
                "Ensemble predictor dimensions");
        for(int j=0;j<models;j++)
            mse[j]+=std::pow(historical_forecasts[t][j]-
                            historical_realized[t],2)/
                    historical_realized.size();
    }
    double total=0;
    for(double& v:mse){v=1/std::max(v,floor);total+=v;}
    for(double& v:mse)v/=total;
    return mse;
}
inline double weighted_model_forecast(const std::vector<double>& weights,
                                      const std::vector<double>& forecasts){
    check_finite(weights);check_finite(forecasts);
    require(weights.size()==forecasts.size(),"Ensemble forecast dimensions");
    double result=0;
    for(int j=0;j<int(weights.size());j++)result+=weights[j]*forecasts[j];
    return result;
}
} // namespace kline
