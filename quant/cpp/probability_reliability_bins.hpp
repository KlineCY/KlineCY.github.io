#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct ReliabilityBin{int count=0;double mean_forecast=0,event_rate=0;};
inline std::vector<ReliabilityBin> binary_reliability_bins(
    const std::vector<double>& probabilities,const std::vector<int>& labels,
    int bins){
    check_finite(probabilities);
    require(probabilities.size()==labels.size()&&bins>=2&&bins<=1000,
            "Invalid probability calibration bins");
    std::vector<ReliabilityBin> result(bins);
    for(int i=0;i<int(labels.size());i++){
        double p=probabilities[i];
        require(p>=0&&p<=1&&(labels[i]==0||labels[i]==1),
                "Invalid probability or binary outcome");
        int bin=std::min(bins-1,int(p*bins));
        result[bin].count++;
        result[bin].mean_forecast+=p;
        result[bin].event_rate+=labels[i];
    }
    for(auto& bucket:result)if(bucket.count>0){
        bucket.mean_forecast/=bucket.count;
        bucket.event_rate/=bucket.count;
    }
    return result;
}
} // namespace kline
