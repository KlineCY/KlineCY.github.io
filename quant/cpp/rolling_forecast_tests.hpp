#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ForecastComparison{double mean_loss_difference=0;double hac_se=0;double t=0;};
inline ForecastComparison diebold_mariano_squared_error(
    const std::vector<double>& forecast_a,
    const std::vector<double>& forecast_b,
    const std::vector<double>& realized,int hac_lags){
    check_finite(forecast_a);check_finite(forecast_b);check_finite(realized);
    require(forecast_a.size()==realized.size()&&
            forecast_b.size()==realized.size()&&hac_lags>=0&&
            hac_lags<int(realized.size()),"Forecast comparison dimensions");
    std::vector<double> difference;
    for(int i=0;i<int(realized.size());i++)
        difference.push_back(std::pow(realized[i]-forecast_a[i],2)-
                             std::pow(realized[i]-forecast_b[i],2));
    double estimated=mean(difference),variance_hac=0;
    int n=int(difference.size());
    for(double v:difference)variance_hac+=std::pow(v-estimated,2)/n;
    for(int lag=1;lag<=hac_lags;lag++){
        double cov=0;
        for(int t=lag;t<n;t++)
            cov+=(difference[t]-estimated)*(difference[t-lag]-estimated)/n;
        variance_hac+=2*(1-double(lag)/(hac_lags+1))*cov;
    }
    require(variance_hac>0,"Forecast loss difference has zero HAC variance");
    double se=std::sqrt(variance_hac/n);
    return {estimated,se,estimated/se};
}
} // namespace kline
