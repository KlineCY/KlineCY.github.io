#pragma once
#include "risk.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> causal_drawdown_leverage(
    const std::vector<double>& equity,double limit,double gross_cap){
    check_finite(equity);
    require(std::isfinite(limit)&&limit>0&&limit<1&&
            std::isfinite(gross_cap)&&gross_cap>=0,
            "Invalid drawdown risk controls");
    double peak=equity[0];
    std::vector<double> allowed(equity.size());
    for(int i=0;i<int(equity.size());i++){
        require(equity[i]>0,"Drawdown controls need positive equity");
        peak=std::max(peak,equity[i]);
        double drawdown=1-equity[i]/peak;
        allowed[i]=gross_cap*std::max(0.0,1-drawdown/limit);
    }
    return allowed;
}
} // namespace kline
