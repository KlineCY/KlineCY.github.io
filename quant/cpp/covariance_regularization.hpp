#pragma once
#include "numerical.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline Matrix eigenvalue_floor_covariance(const Matrix& covariance,
                                           double fraction=1e-5){
    require(std::isfinite(fraction)&&fraction>=0,"Invalid spectral floor");
    auto eigen=symmetric_eigen(covariance);
    double largest=std::max(0.0,eigen.values.front());
    double floor=largest*fraction;
    int n=int(covariance.size());
    Matrix result(n,std::vector<double>(n,0));
    for(int k=0;k<n;k++){
        double value=std::max(floor,eigen.values[k]);
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                result[i][j]+=value*eigen.vectors[i][k]*eigen.vectors[j][k];
    }
    return result;
}
inline Matrix covariance_correlation(const Matrix& covariance){
    check_covariance(covariance);
    int n=int(covariance.size());
    Matrix corr(n,std::vector<double>(n,0));
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++){
            double denom=std::sqrt(covariance[i][i]*covariance[j][j]);
            corr[i][j]=denom>0?covariance[i][j]/denom:0;
            if(i==j&&denom>0)corr[i][i]=1;
        }
    return corr;
}
inline double covariance_condition_regularized(const Matrix& covariance,
                                                double fraction){
    return condition_number_symmetric(eigenvalue_floor_covariance(covariance,fraction));
}
} // namespace kline
