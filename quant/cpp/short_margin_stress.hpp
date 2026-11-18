#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double short_equity_after_price_move(
    double initial_cash,double short_shares,double initial_sale_price,
    double new_price,double accrued_borrow_fee){
    require(std::isfinite(initial_cash)&&initial_cash>=0&&
            std::isfinite(short_shares)&&short_shares>=0&&
            std::isfinite(initial_sale_price)&&initial_sale_price>0&&
            std::isfinite(new_price)&&new_price>0&&
            std::isfinite(accrued_borrow_fee)&&accrued_borrow_fee>=0,
            "Invalid short margin equity");
    return initial_cash+short_shares*initial_sale_price-
           short_shares*new_price-accrued_borrow_fee;
}
inline double short_maintenance_margin_ratio(
    double equity,double short_shares,double new_price){
    require(std::isfinite(equity)&&std::isfinite(short_shares)&&
            short_shares>0&&std::isfinite(new_price)&&new_price>0,
            "Invalid short margin denominator");
    return equity/(short_shares*new_price);
}
inline bool short_margin_call_required(
    double equity,double short_shares,double stock_price,
    double required_ratio){
    require(std::isfinite(required_ratio)&&required_ratio>=0,
            "Invalid short maintenance margin requirement");
    return short_maintenance_margin_ratio(
        equity,short_shares,stock_price)<required_ratio;
}
} // namespace kline
