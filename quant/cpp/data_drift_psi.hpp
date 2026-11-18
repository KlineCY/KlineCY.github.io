#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double population_stability_index(
    const std::vector<double>& historic,const std::vector<double>& current,
    int bins=10,double floor=1e-6){
    check_finite(historic);check_finite(current);
    require(bins>=2&&bins<=100&&std::isfinite(floor)&&floor>0,
            "Invalid PSI bin count");
    auto sorted=historic;
    std::sort(sorted.begin(),sorted.end());
    std::vector<double> boundaries;
    for(int i=1;i<bins;i++)boundaries.push_back(quantile(sorted,double(i)/bins));
    std::vector<double> hist(bins,0),curr(bins,0);
    for(double v:historic){
        int j=int(std::upper_bound(boundaries.begin(),boundaries.end(),v)-
                  boundaries.begin());
        hist[j]++;
    }
    for(double v:current){
        int j=int(std::upper_bound(boundaries.begin(),boundaries.end(),v)-
                  boundaries.begin());
        curr[j]++;
    }
    double result=0;
    for(int i=0;i<bins;i++){
        double p=std::max(floor,hist[i]/historic.size());
        double q=std::max(floor,curr[i]/current.size());
        result+=(q-p)*std::log(q/p);
    }
    return result;
}
} // namespace kline
