#pragma once
#include "risk.hpp"
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double omega_ratio(const std::vector<double>& returns,double hurdle=0) {
    check_finite(returns);
    require(std::isfinite(hurdle),"Omega hurdle must be finite");
    double gains=0,losses=0;
    for(double r:returns){
        gains+=std::max(0.0,r-hurdle);
        losses+=std::max(0.0,hurdle-r);
    }
    require(losses>0,"Omega ratio needs returns below hurdle");
    return gains/losses;
}

inline double calmar_ratio(const std::vector<double>& returns,
                            int periods=252) {
    require(periods>0,"Calmar annualization must be positive");
    double growth=1;
    std::vector<double> equity={1};
    for(double r:returns){
        require(std::isfinite(r)&&r>-1,"Invalid equity return");
        growth*=1+r;equity.push_back(growth);
    }
    double drawdown=-drawdown_from_equity(equity).worst;
    require(drawdown>0,"Calmar ratio requires a drawdown");
    return cagr(returns,periods)/drawdown;
}

inline double tail_ratio(const std::vector<double>& returns,
                          double tail_fraction=0.05) {
    check_finite(returns);
    require(std::isfinite(tail_fraction)&&tail_fraction>0&&
            tail_fraction<0.5,"Invalid return tail fraction");
    double upper=quantile(returns,1-tail_fraction);
    double lower=quantile(returns,tail_fraction);
    require(lower<0,"Tail ratio requires a negative lower tail");
    return upper/std::fabs(lower);
}

inline double ulcer_index(const std::vector<double>& equity) {
    check_finite(equity);
    double peak=equity[0],sum=0;
    for(double e:equity){
        require(e>0,"Ulcer index needs positive equity");
        peak=std::max(peak,e);
        double drawdown=e/peak-1;
        sum+=drawdown*drawdown;
    }
    return std::sqrt(sum/equity.size());
}

inline double profit_factor(const std::vector<double>& trade_pnl) {
    check_finite(trade_pnl);
    double wins=0,losses=0;
    for(double pnl:trade_pnl){
        wins+=std::max(0.0,pnl);losses+=std::max(0.0,-pnl);
    }
    require(losses>0,"Profit factor requires losing trades");
    return wins/losses;
}
} // namespace kline
