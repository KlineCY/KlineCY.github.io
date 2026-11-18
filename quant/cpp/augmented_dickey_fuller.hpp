#pragma once
#include "ols_models.hpp"
#include "matrix_algorithms.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct AdfRegression{
    double level_coefficient=0,t_statistic=0;
    double residual_variance=0;int usable_observations=0;
};
inline AdfRegression augmented_dickey_fuller(
  const std::vector<double>& levels,int lagged_differences){
    check_finite(levels);
    int n=int(levels.size());
    require(lagged_differences>=0&&
            n>=3*lagged_differences+10,
            "ADF sample too short for specified lagged differences");
    std::vector<double> delta;
    for(int i=1;i<n;i++)delta.push_back(levels[i]-levels[i-1]);
    Matrix x;std::vector<double> y;
    for(int t=lagged_differences+1;t<n;t++){
        std::vector<double> row={levels[t-1]};
        for(int j=1;j<=lagged_differences;j++)
            row.push_back(delta[t-1-j]);
        x.push_back(row);y.push_back(delta[t-1]);
    }
    auto fit=ordinary_least_squares(x,y);
    int p=int(x[0].size())+1;
    Matrix xx(p,std::vector<double>(p));
    for(const auto& row:x){
        std::vector<double> v={1};v.insert(v.end(),row.begin(),row.end());
        for(int i=0;i<p;i++)for(int j=0;j<p;j++)xx[i][j]+=v[i]*v[j];
    }
    std::vector<double> e(p,0);e[1]=1;
    auto column=solve_linear_system(xx,e);
    double standard_error=std::sqrt(fit.mse*column[1]);
    require(standard_error>0,"ADF standard error undefined");
    return {fit.coefficients[1],fit.coefficients[1]/standard_error,
            fit.mse,int(y.size())};
}
} // namespace kline
