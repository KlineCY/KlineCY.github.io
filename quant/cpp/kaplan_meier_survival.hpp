#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace kline {
struct SurvivalStep{double time=0,survival=1;int at_risk=0,defaults=0;};
inline std::vector<SurvivalStep> kaplan_meier(
    const std::vector<double>& durations,const std::vector<int>& event){
    check_finite(durations);
    require(durations.size()==event.size(),
            "Survival time and censor indicator lengths differ");
    std::vector<std::pair<double,int>> data;
    for(int i=0;i<int(durations.size());i++){
        require(durations[i]>=0&&(event[i]==0||event[i]==1),
                "Invalid survival observation");
        data.push_back({durations[i],event[i]});
    }
    std::sort(data.begin(),data.end());
    std::vector<SurvivalStep> result;
    double survival=1;
    int index=0,n=int(data.size());
    while(index<n){
        double time=data[index].first;
        int end=index,defaults=0;
        while(end<n&&data[end].first==time)defaults+=data[end++].second;
        int at_risk=n-index;
        survival*=1-double(defaults)/at_risk;
        result.push_back({time,survival,at_risk,defaults});
        index=end;
    }
    return result;
}
} // namespace kline
