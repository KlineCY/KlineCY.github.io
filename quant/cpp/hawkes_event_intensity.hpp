#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double exponential_hawkes_intensity(
    const std::vector<double>& previous_events,double at_time,
    double baseline,double excitation,double decay){
    require(std::isfinite(at_time)&&std::isfinite(baseline)&&baseline>0&&
            std::isfinite(excitation)&&excitation>=0&&
            std::isfinite(decay)&&decay>0,
            "Invalid Hawkes excitation parameters");
    double result=baseline;
    double previous=-1;
    for(double event:previous_events){
        require(std::isfinite(event)&&event>=0&&event<at_time&&
                event>previous,"Hawkes event ordering");
        result+=excitation*std::exp(-decay*(at_time-event));
        previous=event;
    }
    return result;
}
inline double exponential_hawkes_log_likelihood(
    const std::vector<double>& events,double horizon,
    double baseline,double excitation,double decay){
    require(std::isfinite(horizon)&&horizon>0&&
            std::isfinite(baseline)&&baseline>0&&
            std::isfinite(excitation)&&excitation>=0&&
            std::isfinite(decay)&&decay>0,"Invalid Hawkes likelihood inputs");
    double log_likelihood=0,integral=baseline*horizon;
    std::vector<double> history;
    for(double event:events){
        require(std::isfinite(event)&&event>=0&&event<horizon,
                "Event beyond Hawkes horizon");
        double intensity=exponential_hawkes_intensity(
            history,event,baseline,excitation,decay);
        log_likelihood+=std::log(intensity);
        integral+=excitation/decay*(1-std::exp(-decay*(horizon-event)));
        history.push_back(event);
    }
    return log_likelihood-integral;
}
} // namespace kline
