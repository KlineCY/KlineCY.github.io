#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct HazardCurve{
    std::vector<double> end_times,hazard_rates;
};
inline double survival_probability(const HazardCurve& curve,double years){
    check_finite(curve.end_times);check_finite(curve.hazard_rates);
    require(curve.end_times.size()==curve.hazard_rates.size()&&
            std::isfinite(years)&&years>=0,
            "Invalid piecewise hazard curve");
    double previous=0;
    for(int i=0;i<int(curve.end_times.size());i++){
        require(curve.end_times[i]>previous&&curve.hazard_rates[i]>=0,
                "Hazard times and rates invalid");
        previous=curve.end_times[i];
    }
    double log_survival=0;
    previous=0;
    for(int i=0;i<int(curve.end_times.size());i++){
        require(curve.end_times[i]>previous&&curve.hazard_rates[i]>=0,
                "Hazard times and rates invalid");
        double interval=std::min(years,curve.end_times[i])-previous;
        if(interval>0)log_survival-=interval*curve.hazard_rates[i];
        previous=curve.end_times[i];
        if(years<=previous)return std::exp(log_survival);
    }
    return std::exp(log_survival-(years-previous)*curve.hazard_rates.back());
}
inline double probability_default_by(const HazardCurve& curve,double years){
    return 1-survival_probability(curve,years);
}
inline double expected_discounted_default_loss(
    const HazardCurve& curve,double recovery,double annual_rate,
    double notional,double maturity,int steps=600){
    require(std::isfinite(recovery)&&recovery>=0&&recovery<=1&&
            std::isfinite(annual_rate)&&std::isfinite(notional)&&
            notional>=0&&std::isfinite(maturity)&&maturity>0&&steps>0,
            "Invalid risky loss integration parameters");
    double loss=0,previous_survival=1;
    for(int i=1;i<=steps;i++){
        double t=maturity*i/steps;
        double surviving=survival_probability(curve,t);
        loss+=(previous_survival-surviving)*std::exp(-annual_rate*t);
        previous_survival=surviving;
    }
    return notional*(1-recovery)*loss;
}
} // namespace kline
