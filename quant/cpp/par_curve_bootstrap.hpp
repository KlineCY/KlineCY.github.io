#pragma once
#include "curve_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct DiscountBootstrap{std::vector<double> times,discounts,zero_rates;};
inline DiscountBootstrap bootstrap_annual_par_bonds(
    const std::vector<double>& par_coupon_rates){
    check_finite(par_coupon_rates);
    DiscountBootstrap result;
    double earlier=0;
    for(int i=0;i<int(par_coupon_rates.size());i++){
        double coupon=par_coupon_rates[i];
        require(coupon>-0.5&&coupon<1,"Invalid annual par coupon");
        double final_discount=(1-coupon*earlier)/(1+coupon);
        require(final_discount>0&&std::isfinite(final_discount),
                "Inconsistent par price discount factors");
        double years=i+1.0;
        result.times.push_back(years);
        result.discounts.push_back(final_discount);
        result.zero_rates.push_back(-std::log(final_discount)/years);
        earlier+=final_discount;
    }
    return result;
}
inline double par_coupon_rate_from_discounts(
    const std::vector<double>& annual_discounts){
    check_finite(annual_discounts);
    double annuity=0;
    for(double discount:annual_discounts){
        require(discount>0,"Nonpositive bond discount factor");
        annuity+=discount;
    }
    return (1-annual_discounts.back())/annuity;
}
} // namespace kline
