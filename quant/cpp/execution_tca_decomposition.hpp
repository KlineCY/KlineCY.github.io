#pragma once
#include "trade_fill_reconciliation.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ExecutionAttribution{
    double spread_cost=0,market_movement=0,fees=0,total=0;
};
inline ExecutionAttribution execution_cost_attribution(
    const std::vector<Fill>& fills,
    const std::vector<double>& contemporaneous_midpoints,
    double arrival_mid){
    require(fills.size()==contemporaneous_midpoints.size()&&
            std::isfinite(arrival_mid)&&arrival_mid>0,
            "Invalid execution benchmark alignment");
    check_finite(contemporaneous_midpoints);
    ExecutionAttribution result;
    for(int i=0;i<int(fills.size());i++){
        require(std::isfinite(fills[i].signed_shares)&&
                std::isfinite(fills[i].price)&&fills[i].price>0&&
                std::isfinite(fills[i].commission)&&fills[i].commission>=0&&
                contemporaneous_midpoints[i]>0,
                "Invalid execution fill or midpoint");
        result.spread_cost+=fills[i].signed_shares*
                            (fills[i].price-contemporaneous_midpoints[i]);
        result.market_movement+=fills[i].signed_shares*
                               (contemporaneous_midpoints[i]-arrival_mid);
        result.fees+=fills[i].commission;
    }
    result.total=result.spread_cost+result.market_movement+result.fees;
    return result;
}
} // namespace kline
