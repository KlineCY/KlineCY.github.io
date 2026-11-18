#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct OnlineMomentsState{long long observations=0;double mean_value=0,m2=0;};
inline OnlineMomentsState online_moment_update(OnlineMomentsState state,double value){
    require(std::isfinite(value)&&state.observations>=0&&
            state.observations<1000000000000LL,"Invalid online moment state");
    state.observations++;
    double delta=value-state.mean_value;
    state.mean_value+=delta/state.observations;
    state.m2+=delta*(value-state.mean_value);
    return state;
}
inline double online_moment_sample_variance(const OnlineMomentsState& state){
    require(state.observations>=2&&state.m2>=-1e-9,
            "Sample online variance needs two observations");
    return std::max(0.0,state.m2/(state.observations-1));
}
} // namespace kline
