#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double held_futures_pnl(double old_settlement,double next_settlement,
                               double contracts,double multiplier) {
    require(std::isfinite(old_settlement)&&std::isfinite(next_settlement)&&
            std::isfinite(contracts)&&std::isfinite(multiplier)&&
            multiplier>0,"Invalid held futures inputs");
    return contracts*multiplier*(next_settlement-old_settlement);
}

struct FuturesRoll {
    double close_pnl=0;
    double entry_cost=0;
    double next_contracts=0;
    double next_entry_price=0;
};

inline FuturesRoll roll_futures_position(
    double old_entry,double old_exit,double old_contracts,
    double new_entry,double multiplier,double commission_per_contract) {
    require(std::isfinite(old_entry)&&std::isfinite(old_exit)&&
            std::isfinite(old_contracts)&&std::isfinite(new_entry)&&
            std::isfinite(multiplier)&&multiplier>0&&
            std::isfinite(commission_per_contract)&&commission_per_contract>=0,
            "Invalid futures roll inputs");
    FuturesRoll result;
    result.close_pnl=held_futures_pnl(old_entry,old_exit,old_contracts,multiplier);
    result.entry_cost=2*std::fabs(old_contracts)*commission_per_contract;
    result.next_contracts=old_contracts;
    result.next_entry_price=new_entry;
    return result;
}

inline double futures_basis(double future_price,double spot_price) {
    require(std::isfinite(future_price)&&std::isfinite(spot_price)&&spot_price>0,
            "Invalid futures basis prices");
    return future_price/spot_price-1;
}

inline double annualized_futures_basis(double future_price,double spot_price,
                                       double years) {
    require(std::isfinite(years)&&years>0,"Futures basis maturity must be positive");
    require(future_price>0,"Log futures basis needs positive forward");
    return std::log(1+futures_basis(future_price,spot_price))/years;
}

inline double cash_and_carry_futures(double spot,double financing_rate,
                                     double storage_rate,double income_yield,
                                     double years) {
    require(std::isfinite(spot)&&spot>0&&std::isfinite(financing_rate)&&
            std::isfinite(storage_rate)&&std::isfinite(income_yield)&&
            std::isfinite(years)&&years>=0,"Invalid cost-of-carry inputs");
    return spot*std::exp((financing_rate+storage_rate-income_yield)*years);
}
} // namespace kline
