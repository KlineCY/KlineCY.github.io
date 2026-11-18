#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
struct SeriesQuality {
    int observations=0;
    int missing=0;
    int valid=0;
    int repeated_values=0;
    double coverage=0;
};

inline SeriesQuality inspect_series(const std::vector<double>& values) {
    require(!values.empty(),"Series quality needs observations");
    SeriesQuality result;
    result.observations=int(values.size());
    for(int i=0;i<int(values.size());i++){
        if(!std::isfinite(values[i]))result.missing++;
        else {
            result.valid++;
            if(i>0 && std::isfinite(values[i-1])&&
               values[i]==values[i-1])result.repeated_values++;
        }
    }
    result.coverage=double(result.valid)/result.observations;
    return result;
}

inline std::vector<double> forward_fill(
    const std::vector<double>& values,int maximum_gap) {
    require(!values.empty()&&maximum_gap>=0,
            "Invalid forward-fill gap");
    std::vector<double> out=values;
    double last=0;
    bool known=false;
    int gap=0;
    for(double& value:out){
        if(std::isfinite(value)){
            known=true;last=value;gap=0;
        }else{
            gap++;
            if(known && gap<=maximum_gap)value=last;
        }
    }
    return out;
}

inline std::vector<int> available_rows(const std::vector<double>& values) {
    require(!values.empty(),"No observations to inspect");
    std::vector<int> indices;
    for(int i=0;i<int(values.size());i++)
        if(std::isfinite(values[i]))indices.push_back(i);
    return indices;
}

inline std::vector<double> lagged_signal(
    const std::vector<double>& values,int delay) {
    check_finite(values);
    require(delay>=1&&delay<int(values.size()),
            "Signal delay must be positive and shorter than history");
    std::vector<double> output;
    for(int i=delay;i<int(values.size());i++)
        output.push_back(values[i-delay]);
    return output;
}

inline double max_relative_jump(const std::vector<double>& prices) {
    check_finite(prices);
    require(prices.size()>=2,"Jump check needs prices");
    double largest=0;
    for(int i=1;i<int(prices.size());i++){
        require(prices[i]>0&&prices[i-1]>0,"Jump check needs positive prices");
        largest=std::max(largest,std::fabs(prices[i]/prices[i-1]-1));
    }
    return largest;
}
} // namespace kline
