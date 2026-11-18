#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

namespace kline {
inline std::vector<int> circular_block_indices(
    int observations,int block_length,unsigned int seed) {
    require(observations>=2&&block_length>=1&&block_length<=observations,
            "Invalid circular bootstrap configuration");
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> start(0,observations-1);
    std::vector<int> indices;
    while(int(indices.size())<observations){
        int beginning=start(rng);
        for(int j=0;j<block_length&&int(indices.size())<observations;j++)
            indices.push_back((beginning+j)%observations);
    }
    return indices;
}

inline std::vector<int> stationary_bootstrap_indices(
    int observations,double expected_block,unsigned int seed) {
    require(observations>=2&&std::isfinite(expected_block)&&
            expected_block>=1,"Invalid stationary bootstrap block length");
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> start(0,observations-1);
    std::bernoulli_distribution new_block(1/expected_block);
    std::vector<int> indices;
    int index=start(rng);
    for(int t=0;t<observations;t++){
        if(t>0)index=new_block(rng)?start(rng):(index+1)%observations;
        indices.push_back(index);
    }
    return indices;
}

inline double permutation_p_value_correlation(
    const std::vector<double>& a,const std::vector<double>& b,
    int replicates,unsigned int seed) {
    check_finite(a);check_finite(b);
    require(a.size()==b.size()&&a.size()>=3&&
            replicates>0&&replicates<=200000,
            "Invalid permutation test dimensions");
    double observed=std::fabs(correlation(a,b));
    std::vector<double> shuffled=b;
    std::mt19937 rng(seed);
    int exceed=0;
    for(int repeat=0;repeat<replicates;repeat++){
        std::shuffle(shuffled.begin(),shuffled.end(),rng);
        if(std::fabs(correlation(a,shuffled))>=observed-1e-12)
            exceed++;
    }
    return double(exceed+1)/(replicates+1);
}

inline std::vector<double> delete_one_jackknife_means(
    const std::vector<double>& x) {
    check_finite(x);
    require(x.size()>=2,"Jackknife requires at least two observations");
    double sum=std::accumulate(x.begin(),x.end(),0.0);
    std::vector<double> out;
    for(double value:x)out.push_back((sum-value)/(x.size()-1));
    return out;
}

inline double jackknife_standard_error(const std::vector<double>& estimates) {
    check_finite(estimates);
    require(estimates.size()>=2,"Jackknife needs at least two replicate values");
    double center=mean(estimates),sum=0;
    for(double estimate:estimates)sum+=std::pow(estimate-center,2);
    return std::sqrt((estimates.size()-1.0)/estimates.size()*sum);
}
} // namespace kline
