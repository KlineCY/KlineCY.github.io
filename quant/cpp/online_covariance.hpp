#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct OnlineCovariance{
    int count=0;
    std::vector<double> mean;
    Matrix sum_squares;
};
inline OnlineCovariance new_online_covariance(int variables){
    require(variables>0&&variables<=10000,"Invalid covariance dimension");
    return {0,std::vector<double>(variables,0),
            Matrix(variables,std::vector<double>(variables,0))};
}
inline void update_online_covariance(OnlineCovariance& state,
                                   const std::vector<double>& row){
    check_finite(row);
    require(row.size()==state.mean.size(),"Online covariance row dimensions");
    state.count++;
    int n=int(row.size());
    std::vector<double> delta(n);
    for(int i=0;i<n;i++){
        delta[i]=row[i]-state.mean[i];
        state.mean[i]+=delta[i]/state.count;
    }
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            state.sum_squares[i][j]+=delta[i]*(row[j]-state.mean[j]);
}
inline Matrix finalized_online_covariance(const OnlineCovariance& state){
    require(state.count>=2,"Covariance needs two observations");
    Matrix cov=state.sum_squares;
    for(auto& row:cov)for(double& value:row)value/=(state.count-1);
    return cov;
}
} // namespace kline
