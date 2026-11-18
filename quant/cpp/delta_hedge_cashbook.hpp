#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct DeltaHedgeStep{
    double stock_units=0,cash=0,option_value=0,net_liquidation=0,
           stock_trade=0,financing=0,fees=0;
};
inline std::vector<DeltaHedgeStep> short_option_delta_hedge(
    const std::vector<double>& spots,
    const std::vector<double>& short_option_prices,
    const std::vector<double>& hedge_deltas,double rate_per_period,
    double fee_per_stock_unit){
    check_finite(spots);check_finite(short_option_prices);
    check_finite(hedge_deltas);
    int n=int(spots.size());
    require(n>=2&&short_option_prices.size()==spots.size()&&
            hedge_deltas.size()==spots.size()&&
            std::isfinite(rate_per_period)&&rate_per_period>-1&&
            std::isfinite(fee_per_stock_unit)&&fee_per_stock_unit>=0,
            "Invalid option delta hedge cashbook");
    double stocks=0,cash=short_option_prices[0];
    std::vector<DeltaHedgeStep> result;
    for(int t=0;t<n;t++){
        require(spots[t]>0&&short_option_prices[t]>=0,
                "Nonpositive spot or option mark");
        double financing=0;
        if(t>0){financing=cash*rate_per_period;cash+=financing;}
        double trade=hedge_deltas[t]-stocks;
        double fee=fee_per_stock_unit*std::fabs(trade);
        cash-=trade*spots[t]+fee;stocks=hedge_deltas[t];
        double liquidation=cash+stocks*spots[t]-short_option_prices[t];
        result.push_back({stocks,cash,short_option_prices[t],liquidation,
                          trade,financing,fee});
    }
    return result;
}
} // namespace kline
