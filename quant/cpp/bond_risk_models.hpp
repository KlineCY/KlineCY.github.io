#pragma once
#include "fixed_income_advanced.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ContinuousBondRisk{
    double price=0,duration=0,convexity=0,dv01=0;
};
inline ContinuousBondRisk continuous_bond_risk(
    const std::vector<double>& times,const std::vector<double>& cashflows,
    double yield){
    check_finite(times);check_finite(cashflows);
    require(times.size()==cashflows.size()&&
            std::isfinite(yield),"Invalid bond risk cashflows");
    ContinuousBondRisk r;
    double first=0,second=0;
    for(int i=0;i<int(times.size());i++){
        require(times[i]>0,"Bond times must be future");
        double pv=cashflows[i]*std::exp(-yield*times[i]);
        r.price+=pv;first+=times[i]*pv;second+=times[i]*times[i]*pv;
    }
    require(std::isfinite(r.price)&&r.price>0&&
            std::isfinite(first)&&std::isfinite(second),
            "Bond price, duration or convexity overflow");
    r.duration=first/r.price;
    r.convexity=second/r.price;
    r.dv01=first*1e-4;
    return r;
}
inline double bond_shock_pnl_approx(double price,double duration,
                                   double convexity,double rate_change){
    require(std::isfinite(price)&&price>0&&std::isfinite(duration)&&
            std::isfinite(convexity)&&std::isfinite(rate_change),
            "Invalid rate shock parameters");
    return price*(-duration*rate_change+
                  0.5*convexity*rate_change*rate_change);
}
} // namespace kline
