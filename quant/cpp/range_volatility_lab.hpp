#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
inline void validate_ohlc(const std::vector<double>& open,
                          const std::vector<double>& high,
                          const std::vector<double>& low,
                          const std::vector<double>& close) {
    check_finite(open); check_finite(high); check_finite(low); check_finite(close);
    require(open.size()==high.size() && open.size()==low.size() &&
            open.size()==close.size(),"OHLC columns must align");
    for(std::size_t i=0;i<open.size();++i)
        require(low[i]>0 && open[i]>0 && close[i]>0 &&
                high[i]>=std::max(open[i],close[i]) &&
                low[i]<=std::min(open[i],close[i]) && high[i]>=low[i],
                "Invalid positive OHLC price range");
}
inline double parkinson_intraday_variance(const std::vector<double>& high,
                                          const std::vector<double>& low) {
    check_finite(high);check_finite(low);
    require(high.size()==low.size(),"High and low must align");
    double sum=0;
    for(std::size_t i=0;i<high.size();++i){
        require(low[i]>0&&high[i]>=low[i],"Invalid price range");
        const double x=std::log(high[i]/low[i]);sum+=x*x;
    }
    return sum/(4*std::log(2.0)*high.size());
}
inline double garman_klass_intraday_variance(
    const std::vector<double>& open,const std::vector<double>& high,
    const std::vector<double>& low,const std::vector<double>& close){
    validate_ohlc(open,high,low,close);
    double sum=0;
    for(std::size_t i=0;i<open.size();++i){
        const double range=std::log(high[i]/low[i]);
        const double closeOpen=std::log(close[i]/open[i]);
        sum+=0.5*range*range-(2*std::log(2.0)-1)*closeOpen*closeOpen;
    }
    return std::max(0.0,sum/open.size());
}
inline double rogers_satchell_intraday_variance(
    const std::vector<double>& open,const std::vector<double>& high,
    const std::vector<double>& low,const std::vector<double>& close){
    validate_ohlc(open,high,low,close);
    double sum=0;
    for(std::size_t i=0;i<open.size();++i)
        sum+=std::log(high[i]/close[i])*std::log(high[i]/open[i])+
             std::log(low[i]/close[i])*std::log(low[i]/open[i]);
    return std::max(0.0,sum/open.size());
}
inline double overnight_gap_second_moment(const std::vector<double>& open,
                                           const std::vector<double>& previousClose){
    check_finite(open);check_finite(previousClose);
    require(open.size()==previousClose.size(),"Open and previous close must align");
    double sum=0;
    for(std::size_t i=0;i<open.size();++i){
        require(open[i]>0&&previousClose[i]>0,"Invalid overnight price");
        const double gap=std::log(open[i]/previousClose[i]);sum+=gap*gap;
    }
    return sum/open.size();
}
} // namespace kline
