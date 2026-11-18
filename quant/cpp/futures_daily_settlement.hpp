#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct FuturesSettlementStep{double variation_margin=0,equity=0,
    maintenance_shortfall=0;};
inline std::vector<FuturesSettlementStep> futures_daily_settlement(
    const std::vector<double>& settlement_prices,double contracts,
    double multiplier,double cash_collateral,double maintenance_requirement){
    check_finite(settlement_prices);
    require(settlement_prices.size()>=2&&
            std::isfinite(contracts)&&std::isfinite(multiplier)&&multiplier>0&&
            std::isfinite(cash_collateral)&&cash_collateral>=0&&
            std::isfinite(maintenance_requirement)&&maintenance_requirement>=0,
            "Invalid futures variation margin inputs");
    double equity=cash_collateral;
    std::vector<FuturesSettlementStep> report;
    for(int i=1;i<int(settlement_prices.size());i++){
        double variation=contracts*multiplier*
                         (settlement_prices[i]-settlement_prices[i-1]);
        equity+=variation;
        report.push_back({variation,equity,
                          std::max(0.0,maintenance_requirement-equity)});
    }
    return report;
}
} // namespace kline
