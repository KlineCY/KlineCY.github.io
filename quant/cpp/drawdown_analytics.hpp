#pragma once
#include "risk.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct UnderwaterEpisode{
    int peak=0,trough=0,recovery=-1;
    double deepest=0;
};
inline std::vector<UnderwaterEpisode> drawdown_episodes(
    const std::vector<double>& equity){
    check_finite(equity);for(double v:equity)require(v>0,"Positive equity required");
    double peak=equity[0];int peak_index=0;
    std::vector<UnderwaterEpisode> result;
    UnderwaterEpisode active;bool underwater=false;
    for(int t=1;t<int(equity.size());t++){
        if(equity[t]>=peak){
            if(underwater){active.recovery=t;result.push_back(active);
                           underwater=false;}
            peak=equity[t];peak_index=t;
        }else{
            double dd=equity[t]/peak-1;
            if(!underwater){
                active={peak_index,t,-1,dd};underwater=true;
            }else if(dd<active.deepest){
                active.deepest=dd;active.trough=t;
            }
        }
    }
    if(underwater)result.push_back(active);
    return result;
}
inline double max_drawdown_duration(const std::vector<double>& equity){
    auto episodes=drawdown_episodes(equity);
    int longest=0;
    for(const auto& e:episodes)
        longest=std::max(longest,(e.recovery<0?int(equity.size())-1:e.recovery)
                                  -e.peak);
    return longest;
}
} // namespace kline
