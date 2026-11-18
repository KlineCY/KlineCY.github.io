#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double information_ratio_of_active_returns(
    const std::vector<double>& active_returns,int periods_per_year=252){
    check_finite(active_returns);
    require(active_returns.size()>=2&&periods_per_year>0,
            "Information ratio needs observations and frequency");
    double deviation=std::sqrt(variance(active_returns));
    require(deviation>0,"Information ratio requires nonzero active risk");
    return mean(active_returns)/deviation*std::sqrt(double(periods_per_year));
}
inline double upside_downside_capture(
   const std::vector<double>& fund,const std::vector<double>& benchmark,bool upside){
    check_finite(fund);check_finite(benchmark);
    require(fund.size()==benchmark.size(),"Capture input dates mismatch");
    double fund_sum=0,bench_sum=0;
    for(int i=0;i<int(fund.size());i++)if(upside?benchmark[i]>0:benchmark[i]<0){
        fund_sum+=fund[i];bench_sum+=benchmark[i];
    }
    require(std::fabs(bench_sum)>1e-12,"No matching market capture observations");
    return fund_sum/bench_sum;
}
} // namespace kline
