#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct PageHinkleyObservation{
    int index=0;double cumulative=0;double statistic=0;bool alarm=false;
};
inline std::vector<PageHinkleyObservation> page_hinkley_upper(
    const std::vector<double>& x,double allowance,double threshold){
    check_finite(x);
    require(std::isfinite(allowance)&&allowance>=0&&
            std::isfinite(threshold)&&threshold>0,
            "Invalid Page Hinkley detection parameters");
    double mu=0,cumulative=0,minimum=0;
    std::vector<PageHinkleyObservation> result;
    for(int i=0;i<int(x.size());i++){
        mu+=(x[i]-mu)/(i+1);
        cumulative+=x[i]-mu-allowance;
        minimum=std::min(minimum,cumulative);
        double statistic=cumulative-minimum;
        result.push_back({i,cumulative,statistic,statistic>threshold});
    }
    return result;
}
} // namespace kline
