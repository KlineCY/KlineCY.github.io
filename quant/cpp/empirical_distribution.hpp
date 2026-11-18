#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double empirical_cdf(const std::vector<double>& sample,double x){
    check_finite(sample);
    require(std::isfinite(x),"Nonfinite empirical distribution query");
    auto sorted=sample;std::sort(sorted.begin(),sorted.end());
    return double(std::upper_bound(sorted.begin(),sorted.end(),x)-
                  sorted.begin())/sorted.size();
}
inline double one_sample_ks_statistic(
    const std::vector<double>& sample,
    const std::vector<double>& reference_cdf_values){
    check_finite(sample);check_finite(reference_cdf_values);
    require(sample.size()==reference_cdf_values.size(),
            "KS reference CDF shape");
    std::vector<std::pair<double,double>> pairs;
    for(int i=0;i<int(sample.size());i++){
        require(reference_cdf_values[i]>=0&&reference_cdf_values[i]<=1,
                "Invalid reference CDF value");
        pairs.push_back({sample[i],reference_cdf_values[i]});
    }
    std::sort(pairs.begin(),pairs.end());
    double discrepancy=0;
    int n=int(pairs.size());
    for(int i=0;i<n;i++){
        double F=pairs[i].second;
        discrepancy=std::max(discrepancy,
               std::max(std::fabs((i+1.0)/n-F),std::fabs(double(i)/n-F)));
    }
    return discrepancy;
}
inline double two_sample_ks_statistic(
       const std::vector<double>& a,const std::vector<double>& b){
    check_finite(a);check_finite(b);
    auto x=a,y=b;std::sort(x.begin(),x.end());std::sort(y.begin(),y.end());
    int i=0,j=0;double diff=0;
    while(i<int(x.size())||j<int(y.size())){
        double v=i==int(x.size())?y[j]:
                 j==int(y.size())?x[i]:std::min(x[i],y[j]);
        while(i<int(x.size())&&x[i]<=v)i++;
        while(j<int(y.size())&&y[j]<=v)j++;
        diff=std::max(diff,std::fabs(double(i)/x.size()-double(j)/y.size()));
    }
    return diff;
}
} // namespace kline
