#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct Fill{double signed_shares=0,price=0,commission=0;};
struct FillReconciliation{
    double net_shares=0,gross_shares=0,cash_change=0,commissions=0,
           gross_notional=0;
};
inline FillReconciliation reconcile_fills(const std::vector<Fill>& fills){
    require(!fills.empty(),"Trade reconciliation needs fills");
    FillReconciliation result;
    for(const auto& fill:fills){
        require(std::isfinite(fill.signed_shares)&&
                std::isfinite(fill.price)&&fill.price>0&&
                std::isfinite(fill.commission)&&fill.commission>=0,
                "Invalid execution fill");
        result.net_shares+=fill.signed_shares;
        result.gross_shares+=std::fabs(fill.signed_shares);
        result.gross_notional+=std::fabs(fill.signed_shares)*fill.price;
        result.commissions+=fill.commission;
        result.cash_change-=fill.signed_shares*fill.price+fill.commission;
    }
    return result;
}
} // namespace kline
