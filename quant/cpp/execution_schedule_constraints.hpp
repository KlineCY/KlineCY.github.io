#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct ExecutionClip{
    std::vector<double> child_orders;
    double unfilled=0;
};
inline ExecutionClip bounded_child_schedule(
    double total_shares,const std::vector<double>& desired,
    const std::vector<double>& max_liquidity){
    check_finite(desired);check_finite(max_liquidity);
    require(desired.size()==max_liquidity.size()&&
            std::isfinite(total_shares)&&total_shares>=0,
            "Invalid child order size inputs");
    ExecutionClip result;result.unfilled=total_shares;
    for(int i=0;i<int(desired.size());i++){
        require(desired[i]>=0&&max_liquidity[i]>=0,
                "Invalid buy child order plan");
        double executed=std::min(
            result.unfilled,std::min(desired[i],max_liquidity[i]));
        result.child_orders.push_back(executed);
        result.unfilled-=executed;
    }
    return result;
}
} // namespace kline
