#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double biased_distance_covariance_squared(
    const std::vector<double>& x,const std::vector<double>& y){
    check_finite(x);check_finite(y);
    require(x.size()==y.size()&&x.size()>=3,
            "Distance covariance requires aligned observations");
    int n=int(x.size());
    Matrix a(n,std::vector<double>(n)),b(n,std::vector<double>(n));
    std::vector<double> ra(n,0),rb(n,0);
    double ga=0,gb=0;
    for(int i=0;i<n;i++)for(int j=0;j<n;j++){
        a[i][j]=std::fabs(x[i]-x[j]);b[i][j]=std::fabs(y[i]-y[j]);
        ra[i]+=a[i][j]/n;rb[i]+=b[i][j]/n;
        ga+=a[i][j]/(n*double(n));gb+=b[i][j]/(n*double(n));
    }
    double value=0;
    for(int i=0;i<n;i++)for(int j=0;j<n;j++){
        double ac=a[i][j]-ra[i]-ra[j]+ga;
        double bc=b[i][j]-rb[i]-rb[j]+gb;
        value+=ac*bc/(n*double(n));
    }
    return std::max(0.0,value);
}
} // namespace kline
