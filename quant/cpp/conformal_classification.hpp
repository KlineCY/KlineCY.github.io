#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double binary_conformal_nonconformity_threshold(
  const std::vector<double>& calibrated_probability,
  const std::vector<int>& calibrated_labels,double miscoverage){
    check_finite(calibrated_probability);
    require(calibrated_probability.size()==calibrated_labels.size()&&
            std::isfinite(miscoverage)&&miscoverage>0&&miscoverage<1,
            "Invalid classification conformal calibration");
    std::vector<double> scores;
    for(int i=0;i<int(calibrated_probability.size());i++){
        double p=calibrated_probability[i];
        require(p>=0&&p<=1&&
                (calibrated_labels[i]==0||calibrated_labels[i]==1),
                "Classification calibration probability or label invalid");
        scores.push_back(calibrated_labels[i]?1-p:p);
    }
    std::sort(scores.begin(),scores.end());
    int rank=int(std::ceil((scores.size()+1)*(1-miscoverage)));
    require(rank>=1&&rank<=int(scores.size()),
            "Classification conformal sample too small");
    return scores[rank-1];
}
inline std::vector<int> binary_conformal_set(double positive_probability,
                                              double nonconformity_threshold){
    require(std::isfinite(positive_probability)&&positive_probability>=0&&
            positive_probability<=1&&std::isfinite(nonconformity_threshold)&&
            nonconformity_threshold>=0&&nonconformity_threshold<=1,
            "Invalid conformal candidate set");
    std::vector<int> result;
    if(positive_probability<=nonconformity_threshold)result.push_back(0);
    if(1-positive_probability<=nonconformity_threshold)result.push_back(1);
    return result;
}
} // namespace kline
