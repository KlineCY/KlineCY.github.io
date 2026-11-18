#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double liquidation_days(const std::vector<double>& shares,
    const std::vector<double>& average_daily_volume,double participation){
    check_finite(shares);check_finite(average_daily_volume);
    require(shares.size()==average_daily_volume.size()&&
            std::isfinite(participation)&&participation>0&&participation<=1,
            "Invalid liquidation inputs");
    double days=0;
    for(int i=0;i<int(shares.size());i++){
        require(average_daily_volume[i]>0,"Nonpositive ADV");
        days=std::max(days,std::fabs(shares[i])/
             (participation*average_daily_volume[i]));
    }
    return days;
}
inline double staged_liquidation_cost(const std::vector<double>& quantities,
    const std::vector<double>& spreads,
    const std::vector<double>& participation_rates,
    double impact_coefficient){
    check_finite(quantities);check_finite(spreads);
    check_finite(participation_rates);
    require(quantities.size()==spreads.size()&&
            quantities.size()==participation_rates.size()&&
            std::isfinite(impact_coefficient)&&impact_coefficient>=0,
            "Invalid liquidity cost arrays");
    double total=0;
    for(int i=0;i<int(quantities.size());i++){
        require(spreads[i]>=0&&participation_rates[i]>=0,
                "Negative liquidity cost rate");
        total+=std::fabs(quantities[i])*(spreads[i]+
                 impact_coefficient*std::sqrt(participation_rates[i]));
    }
    return total;
}
} // namespace kline
