#pragma once
#include "curve_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double bond_static_curve_rolldown_pnl(
    const ZeroCurve& curve,const std::vector<double>& cashflow_times,
    const std::vector<double>& cashflows,double holding_years){
    check_finite(cashflow_times);check_finite(cashflows);
    require(cashflow_times.size()==cashflows.size()&&
            std::isfinite(holding_years)&&holding_years>0,
            "Invalid bond rolldown cashflows");
    double original=present_value_cashflows(curve,cashflow_times,cashflows);
    double ending_value=0,received=0;
    for(int i=0;i<int(cashflow_times.size());i++){
        require(cashflow_times[i]>0,"Bond cashflow time must be future");
        if(cashflow_times[i]<=holding_years)received+=cashflows[i];
        else ending_value+=cashflows[i]*
             curve_discount(curve,cashflow_times[i]-holding_years);
    }
    return ending_value+received-original;
}
} // namespace kline
