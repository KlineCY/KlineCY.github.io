#pragma once
#include "validation.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {

struct MeanInterval {
    double estimate = 0;
    double lower = 0;
    double upper = 0;
};

inline MeanInterval block_bootstrap_interval(
    const std::vector<double>& values, int block_length,
    int replicates, unsigned int seed, double confidence=0.95) {
    check_finite(values);
    require(std::isfinite(confidence) && confidence > 0 && confidence < 1,
            "Invalid bootstrap confidence");
    auto samples=block_bootstrap_means(values,block_length,replicates,seed);
    return {mean(values),quantile(samples,(1-confidence)/2),
            quantile(samples,1-(1-confidence)/2)};
}

inline double hac_mean_tstat(const std::vector<double>& values,
                             int lags, double null_mean=0) {
    require(std::isfinite(null_mean),"Null mean must be finite");
    double se=hac_mean_standard_error(values,lags);
    require(se > 0,"HAC statistic needs positive long-run variance");
    return (mean(values)-null_mean)/se;
}

inline std::vector<double> cusum_positive(
    const std::vector<double>& observations,double reference,
    double allowance) {
    check_finite(observations);
    require(std::isfinite(reference) && std::isfinite(allowance) &&
            allowance >= 0,"Invalid CUSUM parameters");
    std::vector<double> result;
    double statistic=0;
    for (double x : observations) {
        statistic=std::max(0.0,statistic+x-reference-allowance);
        result.push_back(statistic);
    }
    return result;
}

inline int first_cusum_alarm(const std::vector<double>& observations,
                             double reference,double allowance,
                             double threshold) {
    require(std::isfinite(threshold) && threshold > 0,
            "CUSUM threshold must be positive");
    auto statistic=cusum_positive(observations,reference,allowance);
    for (int i=0;i<int(statistic.size());i++)
        if (statistic[i] >= threshold) return i;
    return -1;
}

inline std::vector<double> holm_adjusted_p(const std::vector<double>& pvalues) {
    check_finite(pvalues);
    int n=int(pvalues.size());
    for (double value : pvalues)
        require(value >= 0 && value <= 1,"Invalid multiple-test p-value");
    std::vector<int> order(n);
    std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),
                     [&](int a,int b){return pvalues[a]<pvalues[b];});
    std::vector<double> result(n);
    double previous=0;
    for (int i=0;i<n;i++) {
        previous=std::max(previous,
                          std::min(1.0,pvalues[order[i]]*(n-i)));
        result[order[i]]=previous;
    }
    return result;
}

} // namespace kline
