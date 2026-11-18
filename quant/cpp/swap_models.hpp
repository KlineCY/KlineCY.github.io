#pragma once
#include "curve_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double fixed_leg_present_value(const ZeroCurve& curve,
    const std::vector<double>& payment_times,double annual_coupon,double notional){
    check_finite(payment_times);
    require(std::isfinite(annual_coupon)&&std::isfinite(notional)&&notional>0,
            "Invalid swap fixed leg");
    double previous=0,pv=0;
    for(double date:payment_times){
        require(date>previous,"Payment times must increase");
        pv+=notional*annual_coupon*(date-previous)*
            curve_discount(curve,date);
        previous=date;
    }
    return pv;
}
inline double floating_leg_par_value(const ZeroCurve& curve,
                                     double maturity,double notional){
    require(std::isfinite(maturity)&&maturity>0&&
            std::isfinite(notional)&&notional>0,
            "Invalid float leg");
    return notional*(1-curve_discount(curve,maturity));
}
inline double payer_swap_present_value(const ZeroCurve& curve,
    const std::vector<double>& payment_times,double coupon,double notional){
    require(!payment_times.empty(),"Swap needs dates");
    return floating_leg_par_value(curve,payment_times.back(),notional)-
           fixed_leg_present_value(curve,payment_times,coupon,notional);
}
inline double forward_rate_from_discounts(const ZeroCurve& curve,
    double start,double end){
    require(std::isfinite(start)&&start>=0&&std::isfinite(end)&&end>start,
            "Invalid forward rate interval");
    return (curve_discount(curve,start)/curve_discount(curve,end)-1)/(end-start);
}
} // namespace kline
