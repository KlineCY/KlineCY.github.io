#pragma once
#include "probability_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double normal_var_with_liquidation(
    double portfolio_notional,double mean_return,double daily_volatility,
    double confidence,const std::vector<double>& order_notional,
    const std::vector<double>& execution_cost_fraction){
    require(std::isfinite(portfolio_notional)&&portfolio_notional>=0&&
            std::isfinite(mean_return)&&std::isfinite(daily_volatility)&&
            daily_volatility>=0&&std::isfinite(confidence)&&
            confidence>0&&confidence<1,"Invalid market VaR inputs");
    check_finite(order_notional);check_finite(execution_cost_fraction);
    require(order_notional.size()==execution_cost_fraction.size(),
            "Liquidity VaR execution costs mismatch");
    double cost=0;
    for(int i=0;i<int(order_notional.size());i++){
        require(execution_cost_fraction[i]>=0,"Negative liquidation cost");
        cost+=std::fabs(order_notional[i])*execution_cost_fraction[i];
    }
    double market=portfolio_notional*
       (-mean_return+normal_inverse_cdf(confidence)*daily_volatility);
    return std::max(0.0,market)+cost;
}
} // namespace kline
