#pragma once
#include "curve_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double fixed_leg_present_value(
    const ZeroCurve& curve,const std::vector<double>& payment_times,
    const std::vector<double>& accruals,double fixed_rate,double notional){
    check_finite(payment_times);check_finite(accruals);
    require(payment_times.size()==accruals.size()&&
            std::isfinite(fixed_rate)&&std::isfinite(notional)&&notional>0,
            "Invalid fixed swap leg");
    double value=0,previous=0;
    for(int i=0;i<int(payment_times.size());i++){
        require(payment_times[i]>previous&&accruals[i]>0,
                "Swap payment times or accruals invalid");
        previous=payment_times[i];
        value+=notional*fixed_rate*accruals[i]*
               curve_discount(curve,payment_times[i]);
    }
    return value;
}
inline double floating_par_leg_present_value(
    const ZeroCurve& curve,double maturity,double notional){
    require(std::isfinite(maturity)&&maturity>0&&
            std::isfinite(notional)&&notional>0,
            "Invalid par floating leg maturity");
    return notional*(1-curve_discount(curve,maturity));
}
inline double payer_swap_present_value(
    const ZeroCurve& curve,const std::vector<double>& dates,
    const std::vector<double>& accruals,double coupon,double notional){
    require(!dates.empty(),"Swap requires cashflow dates");
    return floating_par_leg_present_value(curve,dates.back(),notional)-
           fixed_leg_present_value(curve,dates,accruals,coupon,notional);
}
} // namespace kline
