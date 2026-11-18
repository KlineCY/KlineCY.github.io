#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct CashflowLedger{
    std::vector<double> equity;
    double total_contribution=0,ending_equity=0;
};
inline CashflowLedger time_weighted_equity(
    double initial_equity,const std::vector<double>& returns,
    const std::vector<double>& cashflows){
    check_finite(returns);check_finite(cashflows);
    require(initial_equity>0&&std::isfinite(initial_equity)&&
            returns.size()==cashflows.size(),"Invalid cashflow ledger");
    CashflowLedger result;
    double equity=initial_equity;
    result.equity.push_back(equity);
    for(int t=0;t<int(returns.size());t++){
        require(returns[t]>-1,"Account lost all NAV");
        equity=equity*(1+returns[t])+cashflows[t];
        require(equity>0&&std::isfinite(equity),"Nonpositive account after cashflow");
        result.equity.push_back(equity);
        result.total_contribution+=cashflows[t];
    }
    result.ending_equity=equity;
    return result;
}
inline double time_weighted_growth(const std::vector<double>& returns){
    check_finite(returns);
    double growth=1;
    for(double r:returns){
        require(r>-1,"Invalid compound return");
        growth*=1+r;
    }
    return growth;
}
inline double money_weighted_irr(
    const std::vector<double>& investor_cashflows){
    check_finite(investor_cashflows);
    require(investor_cashflows.size()>=2,"IRR needs at least two periods");
    auto npv=[&](double rate){
        double total=0;
        for(int t=0;t<int(investor_cashflows.size());t++)
            total+=investor_cashflows[t]/std::pow(1+rate,t);
        return total;
    };
    double lo=-0.9999,hi=1;
    double left=npv(lo),right=npv(hi);
    for(int i=0;i<40&&left*right>0;i++){
        hi=2*hi+1;right=npv(hi);
    }
    require(std::isfinite(left)&&std::isfinite(right)&&left*right<=0,
            "IRR not bracketed or multiple-root ambiguity");
    for(int i=0;i<130;i++){
        double middle=(lo+hi)/2,at=npv(middle);
        if((at>0)==(left>0)){lo=middle;left=at;}
        else hi=middle;
    }
    return (lo+hi)/2;
}
} // namespace kline
