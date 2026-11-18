#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct DrawdownTiming{double largest_fraction=0;int longest_underwater=0;
    int peak_index=0,trough_index=0;};
inline DrawdownTiming drawdown_timing(const std::vector<double>& equity){
    check_finite(equity);
    double running_peak=equity[0];
    require(running_peak>0,"Initial equity must be positive");
    DrawdownTiming result;
    int last_peak=0,underwater=0;
    for(int i=1;i<int(equity.size());i++){
        require(equity[i]>0,"Equity path cannot be nonpositive");
        if(equity[i]>=running_peak){
            running_peak=equity[i];last_peak=i;underwater=0;
        }else{
            underwater++;
            double drawdown=1-equity[i]/running_peak;
            if(drawdown>result.largest_fraction){
                result.largest_fraction=drawdown;
                result.peak_index=last_peak;result.trough_index=i;
            }
            result.longest_underwater=std::max(result.longest_underwater,underwater);
        }
    }
    return result;
}
} // namespace kline
