#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct HACMean{double estimate=0,long_run_variance=0,standard_error=0,t_stat=0;};
inline HACMean bartlett_hac_mean(const std::vector<double>& data,int lags){
    check_finite(data);
    int n=int(data.size());
    require(n>=3&&lags>=0&&lags<n-1,
            "Invalid Newey West mean lag count");
    double mu=mean(data),cov0=0;
    for(double v:data)cov0+=(v-mu)*(v-mu)/n;
    double long_run=cov0;
    for(int lag=1;lag<=lags;lag++){
        double autocov=0;
        for(int t=lag;t<n;t++)autocov+=
           (data[t]-mu)*(data[t-lag]-mu)/n;
        long_run+=2*(1-double(lag)/(lags+1))*autocov;
    }
    require(long_run>=-1e-12,"Negative HAC long run covariance");
    long_run=std::max(0.0,long_run);
    double se=std::sqrt(long_run/n);
    return {mu,long_run,se,se>0?mu/se:0};
}
} // namespace kline
