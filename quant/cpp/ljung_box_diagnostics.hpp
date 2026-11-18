#pragma once
#include "partial_autocorrelation.hpp"
#include <cmath>
namespace kline {
inline double ljung_box_q_statistic(const std::vector<double>& residuals,int lags){
    check_finite(residuals);
    require(lags>=1&&lags<int(residuals.size())-1,"Insufficient Ljung Box lags");
    auto acf=autocorrelations(residuals,lags);
    double n=double(residuals.size()),q=0;
    for(int k=1;k<=lags;k++)q+=acf[k]*acf[k]/(n-k);
    return n*(n+2)*q;
}
} // namespace kline
