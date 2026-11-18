#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double kpss_level_statistic(const std::vector<double>& observations,
                                  int hac_lags){
    check_finite(observations);
    int n=int(observations.size());
    require(n>=6&&hac_lags>=0&&hac_lags<n-1,
            "Invalid KPSS sample or bandwidth");
    double m=mean(observations);
    std::vector<double> residual(n);
    double sum_squares=0;
    for(int i=0;i<n;i++)residual[i]=observations[i]-m;
    double cumulative=0;
    for(double v:residual){cumulative+=v;sum_squares+=cumulative*cumulative;}
    double long_run_variance=0;
    for(double v:residual)long_run_variance+=v*v/n;
    for(int lag=1;lag<=hac_lags;lag++){
        double cov=0;
        for(int i=lag;i<n;i++)cov+=residual[i]*residual[i-lag]/n;
        long_run_variance+=2*(1.0-double(lag)/(hac_lags+1))*cov;
    }
    require(long_run_variance>0,"KPSS variance degenerate");
    return sum_squares/(n*double(n)*long_run_variance);
}
} // namespace kline
