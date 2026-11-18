#pragma once
#include "logistic_irls.hpp"
#include <vector>
namespace kline {
struct PlattFit{LogisticFit model;};
inline PlattFit fit_platt_scaling(const std::vector<double>& scores,
                 const std::vector<int>& labels,double ridge=1){
    check_finite(scores);
    require(scores.size()==labels.size(),"Platt sample dimension");
    Matrix x;
    for(double score:scores)x.push_back({score});
    return {fit_logistic_irls(x,labels,ridge)};
}
inline double platt_probability(const PlattFit& fit,double score){
    require(std::isfinite(score),"Invalid Platt prediction score");
    return logistic_probability(fit.model,{score});
}
} // namespace kline
