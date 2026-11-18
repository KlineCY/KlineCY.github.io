#pragma once
#include "isotonic_real_regression.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> monotone_quantile_predictions(
    const std::vector<double>& scores,
    const std::vector<double>& target,
    const std::vector<double>& new_scores){
    auto isotonic=fit_isotonic_real(scores,target);
    check_finite(new_scores);
    std::vector<double> prediction;
    for(double score:new_scores)
        prediction.push_back(isotonic_real_predict(isotonic,score));
    return prediction;
}
inline double monotonicity_violation(
    const std::vector<double>& inputs,
    const std::vector<double>& outputs){
    check_finite(inputs);check_finite(outputs);
    require(inputs.size()==outputs.size(),"Monotonicity data shape");
    std::vector<std::pair<double,double>> pairs;
    for(int i=0;i<int(inputs.size());i++)
        pairs.push_back({inputs[i],outputs[i]});
    std::sort(pairs.begin(),pairs.end());
    double largest=0;
    for(int i=1;i<int(pairs.size());i++)
        if(pairs[i].first>pairs[i-1].first)
            largest=std::max(largest,pairs[i-1].second-pairs[i].second);
    return largest;
}
} // namespace kline
