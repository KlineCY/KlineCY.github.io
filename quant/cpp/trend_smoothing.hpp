#pragma once
#include "cholesky_numerics.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> second_difference_trend(
    const std::vector<double>& observations,double smoothness){
    check_finite(observations);
    require(observations.size()>=3&&std::isfinite(smoothness)&&smoothness>=0,
            "Trend smoother needs at least three samples and nonnegative penalty");
    int n=int(observations.size());
    Matrix system(n,std::vector<double>(n,0));
    for(int i=0;i<n;i++)system[i][i]=1;
    for(int t=0;t<n-2;t++){
        const double d[]={1,-2,1};
        for(int i=0;i<3;i++)for(int j=0;j<3;j++)
            system[t+i][t+j]+=smoothness*d[i]*d[j];
    }
    return solve_cholesky(system,observations);
}
inline double trend_roughness(const std::vector<double>& values){
    check_finite(values);
    double roughness=0;
    for(int i=2;i<int(values.size());i++)
        roughness+=std::pow(values[i]-2*values[i-1]+values[i-2],2);
    return roughness;
}
} // namespace kline
