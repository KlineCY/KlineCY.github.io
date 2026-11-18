#pragma once
#include "stats.hpp"
#include <cmath>
#include <random>
#include <vector>
namespace kline {
inline double merton_jump_drift_correction(double intensity,
    double jump_log_mean,double jump_log_std){
    require(std::isfinite(intensity)&&intensity>=0&&
            std::isfinite(jump_log_mean)&&
            std::isfinite(jump_log_std)&&jump_log_std>=0,
            "Invalid Merton jump parameters");
    return intensity*(std::exp(jump_log_mean+0.5*jump_log_std*jump_log_std)-1);
}
inline std::vector<double> merton_jump_prices(double initial,double drift,
    double diffusion,double intensity,double jump_mean,double jump_std,
    double dt,int steps,unsigned int seed){
    require(std::isfinite(initial)&&initial>0&&std::isfinite(drift)&&
            std::isfinite(diffusion)&&diffusion>=0&&
            std::isfinite(intensity)&&intensity>=0&&
            std::isfinite(jump_mean)&&std::isfinite(jump_std)&&jump_std>=0&&
            std::isfinite(dt)&&dt>0&&steps>=1&&steps<=1000000,
            "Invalid Merton simulation");
    std::mt19937 rng(seed);
    std::normal_distribution<double> normal(0,1);
    std::poisson_distribution<int> poisson(intensity*dt);
    double correction=merton_jump_drift_correction(intensity,jump_mean,jump_std);
    std::vector<double> path={initial};
    for(int t=0;t<steps;t++){
        int jumps=poisson(rng);
        double aggregate=jumps*jump_mean+std::sqrt(double(jumps))*
                                         jump_std*normal(rng);
        double next=path.back()*std::exp(
            (drift-0.5*diffusion*diffusion-correction)*dt+
            diffusion*std::sqrt(dt)*normal(rng)+aggregate);
        require(next>0&&std::isfinite(next),"Jump simulation overflow");
        path.push_back(next);
    }
    return path;
}
} // namespace kline
