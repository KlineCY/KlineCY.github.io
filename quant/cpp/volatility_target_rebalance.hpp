#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> past_only_volatility_target(
    const std::vector<double>& returns,int lookback,
    double target_annual_volatility,double maximum_gross,
    double periods_per_year=252){
    check_finite(returns);
    require(lookback>=3&&lookback<int(returns.size())&&
            std::isfinite(target_annual_volatility)&&
            target_annual_volatility>=0&&
            std::isfinite(maximum_gross)&&maximum_gross>=0&&
            std::isfinite(periods_per_year)&&periods_per_year>0,
            "Invalid target volatility schedule");
    std::vector<double> target(returns.size(),0);
    for(int t=lookback;t<int(returns.size());t++){
        double average=0;
        for(int k=t-lookback;k<t;k++)
            average+=returns[k]/lookback;
        double squared=0;
        for(int k=t-lookback;k<t;k++)
            squared+=std::pow(returns[k]-average,2)/(lookback-1);
        double annual_vol=std::sqrt(squared*periods_per_year);
        target[t]=annual_vol==0?maximum_gross:
                 std::min(maximum_gross,target_annual_volatility/annual_vol);
    }
    return target;
}
} // namespace kline
