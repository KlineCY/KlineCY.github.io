#pragma once
#include "portfolio.hpp"
#include <cmath>
namespace kline {
inline Matrix shrink_covariance_to_diagonal(const Matrix& raw,double intensity){
    check_covariance(raw);
    require(std::isfinite(intensity)&&intensity>=0&&intensity<=1,
            "Covariance shrinkage must lie in [0,1]");
    Matrix result=raw;
    for(int i=0;i<int(raw.size());i++)
        for(int j=0;j<int(raw.size());j++)
            if(i!=j)result[i][j]*=1-intensity;
    return result;
}
inline double covariance_average_variance(const Matrix& raw){
    check_covariance(raw);
    double total=0;
    for(int i=0;i<int(raw.size());i++)total+=raw[i][i];
    return total/raw.size();
}
} // namespace kline
