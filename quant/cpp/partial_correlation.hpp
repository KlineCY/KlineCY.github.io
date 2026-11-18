#pragma once
#include "cholesky_numerics.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline Matrix precision_from_covariance(const Matrix& covariance,
                                        double diagonal_ridge=1e-8){
    require(std::isfinite(diagonal_ridge)&&diagonal_ridge>=0,
            "Invalid inverse covariance ridge");
    Matrix regularized=covariance;
    require(!regularized.empty(),"Empty covariance input");
    for(int i=0;i<int(regularized.size());i++)
        regularized[i][i]+=diagonal_ridge;
    auto factor=cholesky_factor(regularized);
    (void)factor;
    int n=int(regularized.size());
    Matrix precision(n,std::vector<double>(n));
    for(int j=0;j<n;j++){
        std::vector<double> unit(n,0);unit[j]=1;
        auto solution=solve_cholesky(regularized,unit);
        for(int i=0;i<n;i++)precision[i][j]=solution[i];
    }
    return precision;
}
inline Matrix partial_correlation_matrix(const Matrix& covariance,
                                         double ridge=1e-8){
    auto precision=precision_from_covariance(covariance,ridge);
    int n=int(precision.size());
    Matrix result(n,std::vector<double>(n,1));
    for(int i=0;i<n;i++)for(int j=0;j<i;j++){
        double denominator=std::sqrt(precision[i][i]*precision[j][j]);
        require(denominator>0,"Invalid precision diagonal");
        result[i][j]=result[j][i]=-precision[i][j]/denominator;
    }
    return result;
}
} // namespace kline
