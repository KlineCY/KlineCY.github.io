#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct BarrierLabel{
    int start=0,end=0,direction=0;
    double realized_return=0;
};
inline BarrierLabel label_triple_barrier(const std::vector<double>& prices,
    int start,int horizon,double profit_take,double stop_loss){
    check_finite(prices);
    require(start>=0&&start<int(prices.size())-1&&
            horizon>=1&&std::isfinite(profit_take)&&profit_take>0&&
            std::isfinite(stop_loss)&&stop_loss>0,
            "Invalid triple barrier parameters");
    for(double p:prices)require(p>0,"Triple barrier needs positive prices");
    int last=std::min(int(prices.size())-1,start+horizon);
    double entry=prices[start];BarrierLabel result;
    result.start=start;result.end=last;
    for(int t=start+1;t<=last;t++){
        double r=prices[t]/entry-1;
        if(r>=profit_take){result.end=t;result.direction=1;break;}
        if(r<=-stop_loss){result.end=t;result.direction=-1;break;}
    }
    result.realized_return=prices[result.end]/entry-1;
    return result;
}
inline std::vector<BarrierLabel> label_all_barriers(
    const std::vector<double>& prices,int horizon,
    double profit_take,double stop_loss){
    check_finite(prices);
    std::vector<BarrierLabel> out;
    for(int t=0;t+1<int(prices.size());t++)
        out.push_back(label_triple_barrier(prices,t,horizon,
                                           profit_take,stop_loss));
    return out;
}
} // namespace kline
