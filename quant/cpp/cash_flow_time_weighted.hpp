#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double period_time_weighted_return(
    const std::vector<double>& start_values,
    const std::vector<double>& end_values_before_external_flow){
    check_finite(start_values);check_finite(end_values_before_external_flow);
    require(start_values.size()==end_values_before_external_flow.size(),
            "Time weighted subperiod dimensions differ");
    double compound=1;
    for(int i=0;i<int(start_values.size());i++){
        require(start_values[i]>0&&end_values_before_external_flow[i]>=0,
                "Invalid time weighted portfolio values");
        compound*=end_values_before_external_flow[i]/start_values[i];
    }
    return compound-1;
}
inline double dietz_modified_return(
    double beginning_equity,double ending_equity,
    const std::vector<double>& external_contributions,
    const std::vector<double>& remaining_time_fraction){
    check_finite(external_contributions);check_finite(remaining_time_fraction);
    require(external_contributions.size()==remaining_time_fraction.size()&&
            std::isfinite(beginning_equity)&&beginning_equity>0&&
            std::isfinite(ending_equity)&&ending_equity>=0,
            "Invalid Modified Dietz cashflow report");
    double contribution=0,weighted=beginning_equity;
    for(int i=0;i<int(external_contributions.size());i++){
        require(remaining_time_fraction[i]>=0&&remaining_time_fraction[i]<=1,
                "Contribution weights must be inside period");
        contribution+=external_contributions[i];
        weighted+=external_contributions[i]*remaining_time_fraction[i];
    }
    require(weighted>0,"Modified Dietz invested capital nonpositive");
    return (ending_equity-beginning_equity-contribution)/weighted;
}
} // namespace kline
