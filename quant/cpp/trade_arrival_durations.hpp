#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct ArrivalDurationStats{double average_seconds=0,coefficient_of_variation=0;};
inline ArrivalDurationStats trade_arrival_duration_stats(
    const std::vector<double>& seconds){
    check_finite(seconds);
    require(seconds.size()>=4,"Trade duration statistics need four timestamps");
    std::vector<double> durations;
    for(int i=1;i<int(seconds.size());i++){
        require(seconds[i]>seconds[i-1],"Trade timestamps must increase");
        durations.push_back(seconds[i]-seconds[i-1]);
    }
    double average=mean(durations);
    return {average,std::sqrt(variance(durations,false))/average};
}
} // namespace kline
