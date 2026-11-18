#pragma once
#include "stats.hpp"
#include <cmath>
#include <limits>
#include <vector>
namespace kline {
struct ChangePoint{int index=0;double improvement=0;};
inline ChangePoint best_single_mean_break(const std::vector<double>& x,
                                          int min_segment=4){
    check_finite(x);
    int n=int(x.size());
    require(min_segment>=2&&n>=2*min_segment,
            "Insufficient data for mean break");
    std::vector<double> cumulative(n+1,0),squares(n+1,0);
    for(int i=0;i<n;i++){cumulative[i+1]=cumulative[i]+x[i];
                          squares[i+1]=squares[i]+x[i]*x[i];}
    auto sse=[&](int a,int b){
        double total=cumulative[b]-cumulative[a];
        return std::max(0.0,squares[b]-squares[a]-total*total/(b-a));
    };
    double full=sse(0,n);
    ChangePoint result;result.improvement=-1;
    for(int i=min_segment;i<=n-min_segment;i++){
        double gain=full-sse(0,i)-sse(i,n);
        if(gain>result.improvement){result.index=i;result.improvement=gain;}
    }
    return result;
}
inline std::vector<int> binary_segmentation_mean(
    const std::vector<double>& x,int maximum_breaks,int min_segment,
    double penalty){
    check_finite(x);
    require(maximum_breaks>=0&&min_segment>=2&&
            std::isfinite(penalty)&&penalty>=0,"Invalid break search");
    std::vector<std::pair<int,int>> segments={{0,int(x.size())}};
    std::vector<int> breaks;
    for(int k=0;k<maximum_breaks;k++){
        double best_gain=penalty;int chosen=-1,position=-1;
        for(int s=0;s<int(segments.size());s++){
            int start=segments[s].first,end=segments[s].second;
            if(end-start<2*min_segment)continue;
            std::vector<double> window(x.begin()+start,x.begin()+end);
            auto result=best_single_mean_break(window,min_segment);
            if(result.improvement>best_gain){
                best_gain=result.improvement;chosen=s;position=start+result.index;
            }
        }
        if(chosen<0)break;
        auto previous=segments[chosen];
        segments[chosen]={previous.first,position};
        segments.insert(segments.begin()+chosen+1,{position,previous.second});
        breaks.push_back(position);
    }
    std::sort(breaks.begin(),breaks.end());
    return breaks;
}
} // namespace kline
