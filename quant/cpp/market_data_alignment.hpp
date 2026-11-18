#pragma once
#include "stats.hpp"
#include <algorithm>
#include <map>
#include <vector>
namespace kline {
struct DatedValue{
    int date=0;
    double value=0;
};
inline std::vector<DatedValue> aligned_intersection(
    const std::vector<DatedValue>& a,const std::vector<DatedValue>& b,
    std::vector<double>& paired_b){
    require(!a.empty()&&!b.empty(),"Dated series required");
    auto validate=[](const std::vector<DatedValue>& values){
        for(int i=0;i<int(values.size());i++){
            require(std::isfinite(values[i].value),"Nonfinite dated value");
            if(i>0)require(values[i].date>values[i-1].date,
                            "Dates must be strictly ordered");
        }
    };
    validate(a);validate(b);
    int i=0,j=0;
    std::vector<DatedValue> out;paired_b.clear();
    while(i<int(a.size())&&j<int(b.size())){
        if(a[i].date==b[j].date){
            out.push_back(a[i]);paired_b.push_back(b[j].value);i++;j++;
        }else if(a[i].date<b[j].date)i++;else j++;
    }
    require(!out.empty(),"No overlapping market dates");
    return out;
}
inline std::vector<double> asof_last_known(
    const std::vector<DatedValue>& published,
    const std::vector<int>& decision_dates){
    require(!published.empty()&&!decision_dates.empty(),"Missing asof inputs");
    for(int i=1;i<int(published.size());i++)
        require(published[i].date>published[i-1].date,
                "Publication sequence not sorted");
    std::vector<double> out;
    int j=-1;
    for(int i=0;i<int(decision_dates.size());i++){
        if(i>0)require(decision_dates[i]>=decision_dates[i-1],
                       "Decisions must be chronological");
        while(j+1<int(published.size())&&
              published[j+1].date<=decision_dates[i])j++;
        require(j>=0,"No known value at decision time");
        require(std::isfinite(published[j].value),"Nonfinite asof value");
        out.push_back(published[j].value);
    }
    return out;
}
} // namespace kline
