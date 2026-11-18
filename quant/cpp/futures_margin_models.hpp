#pragma once
#include "futures_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct MarginAccount{
    double equity=0,free_cash=0,initial_margin=0,maintenance_margin=0;
    double cumulative_pnl=0;
};
inline MarginAccount open_margin_account(double equity,double contracts,
    double entry_price,double contract_multiplier,double initial_fraction,
    double maintenance_fraction){
    require(std::isfinite(equity)&&equity>0&&std::isfinite(contracts)&&
            std::isfinite(entry_price)&&entry_price>0&&
            std::isfinite(contract_multiplier)&&contract_multiplier>0&&
            std::isfinite(initial_fraction)&&initial_fraction>0&&
            initial_fraction<=1&&std::isfinite(maintenance_fraction)&&
            maintenance_fraction>0&&maintenance_fraction<=initial_fraction,
            "Invalid margin account parameters");
    double notional=std::fabs(contracts)*entry_price*contract_multiplier;
    double initial=notional*initial_fraction;
    require(equity>=initial,"Insufficient initial collateral");
    return {equity,equity-initial,initial,
            notional*maintenance_fraction,0};
}
inline MarginAccount mark_margin_account(MarginAccount account,
    double contracts,double old_settlement,double new_settlement,
    double multiplier,double initial_fraction,double maintenance_fraction){
    double pnl=held_futures_pnl(old_settlement,new_settlement,contracts,multiplier);
    account.equity+=pnl;account.cumulative_pnl+=pnl;
    require(account.equity>0,"Futures account equity exhausted");
    double notional=std::fabs(contracts)*new_settlement*multiplier;
    require(notional>=0&&initial_fraction>=maintenance_fraction&&
            maintenance_fraction>0,"Invalid futures maintenance");
    account.initial_margin=notional*initial_fraction;
    account.maintenance_margin=notional*maintenance_fraction;
    account.free_cash=account.equity-account.initial_margin;
    return account;
}
inline bool margin_call(const MarginAccount& account){
    return account.equity<account.maintenance_margin;
}
} // namespace kline
