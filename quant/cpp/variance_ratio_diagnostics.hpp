#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double overlapping_variance_ratio(const std::vector<double>& increments,int horizon){
    check_finite(increments);
    int n=int(increments.size());
    require(horizon>=2&&horizon<n/2,"Variance ratio horizon must be below half sample");
    double center=mean(increments),one_period=0;
    for(double v:increments)one_period+=(v-center)*(v-center);
    one_period/=n;
    require(one_period>0,"Constant increments have undefined variance ratio");
    double multi=0;
    for(int t=horizon-1;t<n;t++){
        double sum=0;
        for(int j=0;j<horizon;j++)sum+=increments[t-j]-center;
        multi+=sum*sum;
    }
    multi/=n-horizon+1;
    return multi/(horizon*one_period);
}
inline double overlapping_variance_ratio_deviation(const std::vector<double>& increments,int horizon){
    return overlapping_variance_ratio(increments,horizon)-1;
}
} // namespace kline
