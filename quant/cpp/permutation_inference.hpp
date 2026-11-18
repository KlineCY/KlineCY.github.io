#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>
namespace kline {
struct PermutationDifference{
    double observed=0,p_value=0;
};
inline PermutationDifference permutation_mean_difference(
       const std::vector<double>& treatment,const std::vector<double>& control,
       int replicates,unsigned int seed){
    check_finite(treatment);check_finite(control);
    require(replicates>=50&&replicates<=1000000,
            "Invalid randomization replicate count");
    double observed=mean(treatment)-mean(control);
    std::vector<double> all=treatment;
    all.insert(all.end(),control.begin(),control.end());
    int n=int(treatment.size()),extreme=0;
    std::mt19937 rng(seed);
    for(int run=0;run<replicates;run++){
        std::shuffle(all.begin(),all.end(),rng);
        double treated=0,untreated=0;
        for(int i=0;i<int(all.size());i++){
            if(i<n)treated+=all[i]/n;
            else untreated+=all[i]/control.size();
        }
        if(std::fabs(treated-untreated)>=std::fabs(observed)-1e-12)
            extreme++;
    }
    return {observed,double(extreme+1)/(replicates+1)};
}
} // namespace kline
