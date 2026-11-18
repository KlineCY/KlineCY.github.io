#pragma once
#include "state_models.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<KalmanState> rts_random_walk_smoother(
    const std::vector<double>& measurements,KalmanState initial,
    double process_variance,double measurement_variance){
    auto filtered=kalman_filter(measurements,initial,process_variance,
                                measurement_variance);
    require(std::isfinite(process_variance)&&process_variance>=0,
            "Invalid smoothing process variance");
    auto smooth=filtered;
    for(int t=int(filtered.size())-2;t>=0;t--){
        double prediction_var=filtered[t].variance+process_variance;
        require(prediction_var>0,"Zero smoothing prediction variance");
        double gain=filtered[t].variance/prediction_var;
        smooth[t].state=filtered[t].state+
             gain*(smooth[t+1].state-filtered[t].state);
        smooth[t].variance=filtered[t].variance+
             gain*gain*(smooth[t+1].variance-prediction_var);
        require(smooth[t].variance>=-1e-12,"Negative smoothed variance");
        smooth[t].variance=std::max(0.0,smooth[t].variance);
    }
    return smooth;
}
} // namespace kline
