#pragma once
#include "factors.hpp"
#include "ols_models.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
inline std::vector<double> multi_factor_residuals(
    const std::vector<double>& signal,const Matrix& controls) {
    check_feature_table(controls,signal);
    auto fit=ordinary_least_squares(controls,signal);
    return fit.residuals;
}

inline std::vector<double> cross_sectional_rank_weights(
    const std::vector<double>& factor,double gross=1) {
    check_finite(factor);
    require(factor.size()>=2&&std::isfinite(gross)&&gross>0,
            "Rank portfolio needs two assets and positive gross");
    auto order=ranks(factor);
    double center=(factor.size()+1)*0.5;
    double total=0;
    for(double& value:order){
        value-=center;
        total+=std::fabs(value);
    }
    require(total>0,"All factor scores have the same rank");
    for(double& value:order)value*=gross/total;
    return order;
}

inline double rank_turnover(const std::vector<double>& before,
                            const std::vector<double>& after) {
    check_finite(before);check_finite(after);
    require(before.size()==after.size(),"Rank portfolio dimensions differ");
    double sum=0;
    for(int i=0;i<int(before.size());i++)
        sum+=std::fabs(after[i]-before[i]);
    return 0.5*sum;
}

inline std::vector<double> quantile_bucket_returns(
    const std::vector<double>& scores,
    const std::vector<double>& subsequent_returns,int buckets) {
    check_finite(scores);check_finite(subsequent_returns);
    require(scores.size()==subsequent_returns.size()&&
            buckets>=2&&buckets<=int(scores.size()),
            "Invalid quantile bucket dimensions");
    std::vector<int> order(scores.size());
    std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),
        [&](int a,int b){return scores[a]<scores[b];});
    std::vector<double> sum(buckets,0);
    std::vector<int> count(buckets,0);
    for(int i=0;i<int(order.size());i++){
        int bucket=int((long long)i*buckets/order.size());
        int id=order[i];
        sum[bucket]+=subsequent_returns[id];count[bucket]++;
    }
    for(int j=0;j<buckets;j++){
        require(count[j]>0,"Empty quantile bucket");
        sum[j]/=count[j];
    }
    return sum;
}

inline double factor_monotonicity(
    const std::vector<double>& bucket_returns) {
    check_finite(bucket_returns);
    require(bucket_returns.size()>=3,"Monotonicity needs three buckets");
    std::vector<double> index(bucket_returns.size());
    std::iota(index.begin(),index.end(),1.0);
    return spearman(index,bucket_returns);
}
} // namespace kline
