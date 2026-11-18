#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline double poisson_queue_completion_probability(double arrivalRate,
                                                    double horizon,
                                                    int quantityAhead,int ownQuantity){
    require(std::isfinite(arrivalRate)&&arrivalRate>=0&&std::isfinite(horizon)&&
            horizon>=0&&quantityAhead>=0&&ownQuantity>0&&
            quantityAhead+ownQuantity<=10000&&arrivalRate*horizon<=600,
            "Invalid queue arrival model inputs");
    const double mu=arrivalRate*horizon;
    if(mu==0)return 0;
    const int requirement=quantityAhead+ownQuantity;
    double term=std::exp(-mu),cdf=term;
    for(int k=1;k<requirement;++k){
        term*=mu/k;cdf+=term;
    }
    return std::max(0.0,std::min(1.0,1-cdf));
}
inline std::vector<double> exponential_transient_impact(
    const std::vector<double>& signedOrders,double impactPerUnit,double persistence){
    check_finite(signedOrders);
    require(std::isfinite(impactPerUnit)&&impactPerUnit>=0&&
            std::isfinite(persistence)&&persistence>=0&&persistence<=1,
            "Invalid transient-impact process");
    double state=0;std::vector<double> path;
    path.reserve(signedOrders.size());
    for(double q:signedOrders){
        state=persistence*state+impactPerUnit*q;
        require(std::isfinite(state),"Transient impact overflows");
        path.push_back(state);
    }
    return path;
}
struct SignatureVariance{double finest=0,coarse=0,ratio=0;};
inline SignatureVariance realized_variance_signature(
    const std::vector<double>& prices,int coarseStep){
    check_finite(prices);
    require(prices.size()>=3&&coarseStep>=1&&
            coarseStep<int(prices.size()),"Invalid realized signature grid");
    for(double p:prices)require(p>0,"Positive prices required");
    double fine=0,coarse=0;
    for(std::size_t i=1;i<prices.size();++i){
        const double r=std::log(prices[i]/prices[i-1]);fine+=r*r;
    }
    for(std::size_t i=coarseStep;i<prices.size();i+=coarseStep){
        const double r=std::log(prices[i]/prices[i-coarseStep]);coarse+=r*r;
    }
    return {fine,coarse,fine>0?coarse/fine:0};
}
inline std::vector<double> fixed_volume_signed_imbalance(
    const std::vector<double>& volumes,
    const std::vector<double>& tradeDirections,double bucketVolume){
    check_finite(volumes);check_finite(tradeDirections);
    require(volumes.size()==tradeDirections.size()&&
            std::isfinite(bucketVolume)&&bucketVolume>0,
            "Invalid volume-bucket data");
    double filled=0,signedFilled=0;
    std::vector<double> completed;
    for(std::size_t i=0;i<volumes.size();++i){
        require(volumes[i]>=0&&(tradeDirections[i]==1||tradeDirections[i]==-1),
                "Invalid nonnegative trade volume or sign");
        double remaining=volumes[i];
        while(remaining>0){
            double piece=std::min(remaining,bucketVolume-filled);
            require(piece>0,"Volume bucket has numerical underflow");
            filled+=piece;signedFilled+=piece*tradeDirections[i];remaining-=piece;
            if(filled>=bucketVolume-1e-12*bucketVolume){
                completed.push_back(std::abs(signedFilled)/bucketVolume);
                filled=0;signedFilled=0;
                require(completed.size()<=250000,"Too many volume buckets");
            }
        }
    }
    return completed;
}
} // namespace kline
