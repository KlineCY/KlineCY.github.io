#pragma once
#include "curve_models.hpp"
#include "learning.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double bootstrap_continuous_zero_rate(
    double maturity,double bond_price,
    const std::vector<double>& earlier_times,
    const std::vector<double>& earlier_cashflows,
    double final_cashflow,const ZeroCurve& earlier_curve) {
    require(std::isfinite(maturity)&&maturity>0&&
            std::isfinite(bond_price)&&bond_price>0&&
            std::isfinite(final_cashflow)&&final_cashflow>0,
            "Invalid zero-rate bootstrapping inputs");
    require(earlier_times.size()==earlier_cashflows.size(),
            "Incomplete coupon schedule");
    double previous_coupon_pv=0;
    if(!earlier_times.empty()){
        check_finite(earlier_times);check_finite(earlier_cashflows);
        for(int i=0;i<int(earlier_times.size());i++){
            require(earlier_times[i]>0&&earlier_times[i]<maturity,
                    "Earlier coupons must precede maturity");
            previous_coupon_pv+=earlier_cashflows[i]*
                                  curve_discount(earlier_curve,earlier_times[i]);
        }
    }
    double remaining=bond_price-previous_coupon_pv;
    require(remaining>0,"Discounted final cash flow must remain positive");
    return -std::log(remaining/final_cashflow)/maturity;
}

inline double bond_price_from_yield(
    const std::vector<double>& times,const std::vector<double>& cashflows,
    double continuous_yield) {
    check_finite(times);check_finite(cashflows);
    require(times.size()==cashflows.size()&&std::isfinite(continuous_yield),
            "Invalid bond yield schedule");
    double price=0;
    for(int i=0;i<int(times.size());i++){
        require(times[i]>0,"Bond cashflows must occur after valuation");
        price+=cashflows[i]*std::exp(-continuous_yield*times[i]);
    }
    return price;
}

inline double yield_to_maturity_continuous(
    double price,const std::vector<double>& times,
    const std::vector<double>& cashflows) {
    require(std::isfinite(price)&&price>0,"Bond price must be positive");
    check_finite(cashflows);
    for(double cash:cashflows)require(cash>=0,"This YTM solver requires nonnegative flows");
    require(std::any_of(cashflows.begin(),cashflows.end(),
                        [](double x){return x>0;}),"Bond needs positive flow");
    double lo=-1,hi=1;
    for(int i=0;i<30 && bond_price_from_yield(times,cashflows,lo)<price;i++)
        lo*=2;
    for(int i=0;i<30 && bond_price_from_yield(times,cashflows,hi)>price;i++)
        hi*=2;
    require(std::isfinite(lo)&&std::isfinite(hi)&&
            bond_price_from_yield(times,cashflows,lo)>=price&&
            bond_price_from_yield(times,cashflows,hi)<=price,
            "Could not bracket bond yield");
    for(int i=0;i<150;i++){
        double mid=(lo+hi)*0.5;
        if(bond_price_from_yield(times,cashflows,mid)>price)lo=mid;
        else hi=mid;
    }
    return (lo+hi)*0.5;
}

inline std::vector<double> key_rate_hedge_contracts(
    const Matrix& hedge_key_rate_dv01,
    const std::vector<double>& asset_key_rate_dv01) {
    check_feature_table(hedge_key_rate_dv01,asset_key_rate_dv01);
    require(hedge_key_rate_dv01.size()==hedge_key_rate_dv01[0].size(),
            "Key-rate hedge requires a square sensitivity matrix");
    Matrix system=hedge_key_rate_dv01;
    for(int i=0;i<int(system.size());i++)
        for(int j=0;j<i;j++)std::swap(system[i][j],system[j][i]);
    std::vector<double> target=asset_key_rate_dv01;
    for(double& v:target)v=-v;
    return solve_linear_system(system,target);
}
} // namespace kline
