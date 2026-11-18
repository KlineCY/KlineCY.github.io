#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double weighted_empirical_quantile(
   const std::vector<double>& values,const std::vector<double>& weights,
   double probability){
    check_finite(values);check_finite(weights);
    require(values.size()==weights.size()&&std::isfinite(probability)&&
            probability>=0&&probability<=1,"Invalid weighted quantile inputs");
    std::vector<std::pair<double,double>> pairs;
    double total=0;
    for(int i=0;i<int(values.size());i++){
        require(weights[i]>=0,"Negative quantile weight");
        pairs.push_back({values[i],weights[i]});total+=weights[i];
    }
    require(total>0,"Weighted quantile needs positive total mass");
    std::sort(pairs.begin(),pairs.end());
    double cumulative=0;
    for(const auto& pair:pairs){
        cumulative+=pair.second/total;
        if(cumulative>=probability-1e-14)return pair.first;
    }
    return pairs.back().first;
}
inline double weighted_empirical_tail_mean(
   const std::vector<double>& losses,const std::vector<double>& weights,
   double confidence){
    check_finite(losses);check_finite(weights);
    require(losses.size()==weights.size()&&std::isfinite(confidence)&&
            confidence>0&&confidence<1,
            "Invalid weighted tail mean inputs");
    std::vector<std::pair<double,double>> pairs;
    double total=0;
    for(int i=0;i<int(losses.size());i++){
        require(weights[i]>=0,"Negative tail weight");
        total+=weights[i];pairs.push_back({losses[i],weights[i]});
    }
    require(total>0,"Weighted CVaR requires positive total mass");
    std::sort(pairs.begin(),pairs.end(),
       [](const auto& a,const auto& b){return a.first>b.first;});
    double remain=total*(1-confidence),result=0;
    for(const auto& item:pairs){
        double selected=std::min(remain,item.second);
        result+=selected*item.first;remain-=selected;
        if(remain<=1e-12*total)break;
    }
    return result/(total*(1-confidence));
}
} // namespace kline
