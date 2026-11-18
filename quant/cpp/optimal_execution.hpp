#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<double> almgren_chriss_schedule(
    double shares,int intervals,double temporary_impact,
    double volatility,double risk_aversion){
    require(std::isfinite(shares)&&shares>=0&&intervals>=1&&
            intervals<=10000&&std::isfinite(temporary_impact)&&
            temporary_impact>0&&std::isfinite(volatility)&&volatility>=0&&
            std::isfinite(risk_aversion)&&risk_aversion>=0,
            "Invalid optimal execution parameters");
    if(shares==0)return std::vector<double>(intervals,0);
    double kappa=std::sqrt(risk_aversion*volatility*volatility/
                           temporary_impact);
    require(kappa*intervals<650,"Execution risk scale is numerically excessive");
    std::vector<double> weights(intervals);
    if(kappa<1e-8){
        for(double& v:weights)v=1.0/intervals;
    }else{
        for(int j=0;j<intervals;j++)
            weights[j]=std::sinh(kappa*(intervals-j))-
                       std::sinh(kappa*(intervals-j-1));
        double sum=std::accumulate(weights.begin(),weights.end(),0.0);
        require(sum>0&&std::isfinite(sum),"Execution profile overflow");
        for(double& v:weights)v/=sum;
    }
    for(double& v:weights)v*=shares;
    return weights;
}
inline double quadratic_execution_cost(const std::vector<double>& slices,
                                       double temporary_impact,double fee_per_share){
    check_finite(slices);
    require(temporary_impact>=0&&std::isfinite(temporary_impact)&&
            fee_per_share>=0&&std::isfinite(fee_per_share),
            "Invalid execution costs");
    double total=0;
    for(double q:slices){
        require(q>=0,"Negative child order");
        total+=temporary_impact*q*q+fee_per_share*q;
    }
    return total;
}
} // namespace kline
