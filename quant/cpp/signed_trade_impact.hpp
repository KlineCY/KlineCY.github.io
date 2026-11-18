#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<int> tick_rule_trade_signs(
    const std::vector<double>& prices,int first_sign=1){
    check_finite(prices);
    require(first_sign==1||first_sign==-1,
            "Tick rule initialization must be buy or sell");
    std::vector<int> signs;
    int previous=first_sign;
    for(int i=0;i<int(prices.size());i++){
        require(prices[i]>0,"Nonpositive trade price");
        if(i>0){
            if(prices[i]>prices[i-1])previous=1;
            if(prices[i]<prices[i-1])previous=-1;
        }
        signs.push_back(previous);
    }
    return signs;
}
inline double signed_trade_response(
    const std::vector<double>& prices,const std::vector<int>& signs,
    int horizon){
    check_finite(prices);
    require(prices.size()==signs.size()&&horizon>=1&&
            horizon<int(prices.size()),"Invalid trade response horizon");
    double average=0;
    for(int t=0;t+horizon<int(prices.size());t++){
        require(prices[t]>0&&(signs[t]==1||signs[t]==-1),
                "Invalid trade sign or price");
        average+=signs[t]*(prices[t+horizon]-prices[t])/prices[t]/
                 (prices.size()-horizon);
    }
    return average;
}
} // namespace kline
