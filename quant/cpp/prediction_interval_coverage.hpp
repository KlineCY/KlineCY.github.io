#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct CoverageSummary{double empirical_coverage=0,average_width=0;
    int observations=0;};
inline CoverageSummary interval_coverage_report(
    const std::vector<double>& actual,
    const std::vector<double>& lower,const std::vector<double>& upper){
    check_finite(actual);check_finite(lower);check_finite(upper);
    require(actual.size()==lower.size()&&actual.size()==upper.size(),
            "Prediction intervals must align to actuals");
    int covered=0;double width=0;
    for(int i=0;i<int(actual.size());i++){
        require(lower[i]<=upper[i],"Lower interval exceeds upper interval");
        covered+=actual[i]>=lower[i]&&actual[i]<=upper[i];
        width+=upper[i]-lower[i];
    }
    return {double(covered)/actual.size(),width/actual.size(),
            int(actual.size())};
}
} // namespace kline
