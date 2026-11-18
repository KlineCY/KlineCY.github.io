#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double daily_stock_borrow_fee(
    double shares_short,double stock_price,double annual_borrow_rate,
    int days_per_year=360){
    require(std::isfinite(shares_short)&&shares_short>=0&&
            std::isfinite(stock_price)&&stock_price>0&&
            std::isfinite(annual_borrow_rate)&&annual_borrow_rate>=0&&
            days_per_year>0,"Invalid borrow financing parameters");
    return shares_short*stock_price*annual_borrow_rate/days_per_year;
}
inline double short_dividend_payment(
    double short_shares,double cash_dividend_per_share){
    require(std::isfinite(short_shares)&&short_shares>=0&&
            std::isfinite(cash_dividend_per_share)&&
            cash_dividend_per_share>=0,"Invalid short dividend");
    return short_shares*cash_dividend_per_share;
}
inline double restricted_short_proceeds(
    double shares_short,double price,double haircut_fraction){
    require(std::isfinite(shares_short)&&shares_short>=0&&
            std::isfinite(price)&&price>0&&
            std::isfinite(haircut_fraction)&&haircut_fraction>=0,
            "Invalid short collateral calculation");
    return shares_short*price*(1+haircut_fraction);
}
} // namespace kline
