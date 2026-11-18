#pragma once
#include "resampling_models.hpp"
#include <vector>
namespace kline {
inline std::vector<double> stationary_bootstrap_sample(
    const std::vector<double>& values,double block_length,unsigned int seed){
    check_finite(values);
    auto indices=stationary_bootstrap_indices(
        int(values.size()),block_length,seed);
    std::vector<double> sample;
    sample.reserve(indices.size());
    for(int index:indices)sample.push_back(values[index]);
    return sample;
}
} // namespace kline
