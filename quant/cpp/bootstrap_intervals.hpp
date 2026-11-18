#pragma once
#include "resampling_models.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct BootstrapInterval{double estimate=0,lower=0,upper=0;};
inline BootstrapInterval block_bootstrap_mean_interval(
    const std::vector<double>& observations,int block_length,
    int replicates,double confidence,unsigned int seed){
    check_finite(observations);
    require(observations.size()>=3&&block_length>0&&
            block_length<=int(observations.size())&&
            replicates>=50&&replicates<=200000&&
            std::isfinite(confidence)&&confidence>0&&confidence<1,
            "Invalid block bootstrap interval");
    std::vector<double> results;
    for(int i=0;i<replicates;i++){
        auto index=circular_block_indices(int(observations.size()),
                                          block_length,seed+unsigned(i));
        double average=0;
        for(int j:index)average+=observations[j]/observations.size();
        results.push_back(average);
    }
    double tail=(1-confidence)/2;
    return {mean(observations),quantile(results,tail),quantile(results,1-tail)};
}
} // namespace kline
