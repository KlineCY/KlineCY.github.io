#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline Matrix ewma_multivariate_covariance(const Matrix& returns,
                                            double decay){
    require(!returns.empty()&&!returns[0].empty()&&
            std::isfinite(decay)&&decay>=0&&decay<1,
            "Invalid EWMA covariance decay or observations");
    int n=int(returns[0].size());
    Matrix cov(n,std::vector<double>(n,0));
    for(const auto& r:returns){
        check_finite(r);require(int(r.size())==n,
                               "EWMA covariance ragged assets");
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                cov[i][j]=decay*cov[i][j]+(1-decay)*r[i]*r[j];
    }
    return cov;
}
inline double ewma_risk_forecast(const Matrix& ewma_covariance,
                                 const std::vector<double>& weights){
    return std::sqrt(portfolio_variance(ewma_covariance,weights));
}
} // namespace kline
