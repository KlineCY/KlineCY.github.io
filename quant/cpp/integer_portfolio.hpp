#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ShareAllocation{
    std::vector<long long> shares;
    double cash=0,invested=0,commission=0;
};
inline ShareAllocation budgeted_whole_share_allocation(
    double capital,const std::vector<double>& target,
    const std::vector<double>& prices,double commission_per_share=0){
    check_finite(target);check_finite(prices);
    require(target.size()==prices.size()&&std::isfinite(capital)&&capital>0&&
            std::isfinite(commission_per_share)&&commission_per_share>=0,
            "Invalid share allocation");
    double sum=0;for(double w:target){require(w>=0,"Negative long-only allocation");sum+=w;}
    require(sum<=1+1e-9,"Weights exceed available capital");
    ShareAllocation out;out.shares.resize(prices.size());
    out.cash=capital;
    for(int i=0;i<int(prices.size());i++){
        require(prices[i]>0,"Nonpositive asset share price");
        double per_share=prices[i]+commission_per_share;
        long long count=(long long)std::floor(capital*target[i]/per_share+1e-12);
        out.shares[i]=count;
        double cost=count*per_share;
        out.cash-=cost;out.invested+=count*prices[i];
        out.commission+=count*commission_per_share;
    }
    require(out.cash>=-1e-7*capital,"Share order exceeds buying power");
    out.cash=std::max(0.0,out.cash);
    return out;
}
} // namespace kline
