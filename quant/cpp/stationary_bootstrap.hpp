#pragma once
#include "stats.hpp"
#include <random>
#include <vector>
namespace kline {
inline std::vector<int> stationary_bootstrap_indices(
    int observations,double expected_block_length,
    unsigned int seed){
    require(observations>=2&&std::isfinite(expected_block_length)&&
            expected_block_length>=1,"Invalid stationary bootstrap length");
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> draw(0,observations-1);
    std::bernoulli_distribution new_block(1/expected_block_length);
    std::vector<int> indices;indices.reserve(observations);
    int current=draw(rng);
    for(int t=0;t<observations;t++){
        if(t>0){
            if(new_block(rng))current=draw(rng);
            else current=(current+1)%observations;
        }
        indices.push_back(current);
    }
    return indices;
}
inline std::vector<double> stationary_bootstrap_sample(
    const std::vector<double>& values,double block_length,unsigned int seed){
    check_finite(values);
    auto ids=stationary_bootstrap_indices(int(values.size()),block_length,seed);
    std::vector<double> sample;
    for(int id:ids)sample.push_back(values[id]);
    return sample;
}
} // namespace kline
