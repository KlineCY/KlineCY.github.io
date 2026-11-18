#pragma once
#include "weighted_qr_regression.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline QrFit fit_exponential_decay_ols(
    const Matrix& x,const std::vector<double>& y,double half_life){
    check_feature_table(x,y);
    require(std::isfinite(half_life)&&half_life>0,
            "Invalid sample decay half life");
    std::vector<double> weights;
    int n=int(y.size());
    for(int t=0;t<n;t++)
        weights.push_back(std::exp(-std::log(2.0)*(n-1-t)/half_life));
    return weighted_qr_intercept(x,y,weights);
}
} // namespace kline
