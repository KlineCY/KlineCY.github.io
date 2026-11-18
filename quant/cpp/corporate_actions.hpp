#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double stock_total_return(double old_price,double new_price,
                                  double cash_dividend,double split_ratio=1){
    require(old_price>0&&new_price>0&&split_ratio>0&&
            std::isfinite(old_price)&&std::isfinite(new_price)&&
            std::isfinite(cash_dividend)&&cash_dividend>=0&&
            std::isfinite(split_ratio),"Invalid corporate action");
    return (new_price*split_ratio+cash_dividend)/old_price-1;
}
inline std::vector<double> stock_total_returns(
    const std::vector<double>& close,
    const std::vector<double>& dividend,
    const std::vector<double>& split){
    check_finite(close);check_finite(dividend);check_finite(split);
    require(close.size()==dividend.size()&&close.size()==split.size()&&
            close.size()>=2,"Corporate action series shapes");
    std::vector<double> results;
    for(int t=1;t<int(close.size());t++)
        results.push_back(stock_total_return(close[t-1],close[t],
                                             dividend[t],split[t]));
    return results;
}
inline double rights_adjusted_theoretical_ex_price(double price_before,
    double subscription_price,double new_shares_per_old){
    require(std::isfinite(price_before)&&price_before>0&&
            std::isfinite(subscription_price)&&subscription_price>=0&&
            std::isfinite(new_shares_per_old)&&new_shares_per_old>=0,
            "Invalid rights issue");
    return (price_before+new_shares_per_old*subscription_price)/
            (1+new_shares_per_old);
}
} // namespace kline
