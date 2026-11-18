#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct VaRBacktest{
    int observations=0,exceptions=0;
    double expected_exception_rate=0,realized_exception_rate=0;
    double kupiec_lr=0;
};
inline VaRBacktest kupiec_var_coverage(
    const std::vector<double>& realized_losses,
    const std::vector<double>& var_forecasts,double confidence){
    check_finite(realized_losses);check_finite(var_forecasts);
    require(realized_losses.size()==var_forecasts.size()&&
            std::isfinite(confidence)&&confidence>0&&confidence<1,
            "Invalid VaR coverage inputs");
    VaRBacktest r;
    r.observations=int(realized_losses.size());
    r.expected_exception_rate=1-confidence;
    for(int i=0;i<r.observations;i++)
        if(realized_losses[i]>var_forecasts[i])r.exceptions++;
    r.realized_exception_rate=double(r.exceptions)/r.observations;
    double p=r.expected_exception_rate,q=r.realized_exception_rate;
    double null_ll=r.exceptions*std::log(p)+
        (r.observations-r.exceptions)*std::log1p(-p);
    double alt_ll=0;
    if(r.exceptions>0)alt_ll+=r.exceptions*std::log(q);
    if(r.exceptions<r.observations)
        alt_ll+=(r.observations-r.exceptions)*std::log1p(-q);
    r.kupiec_lr=std::max(0.0,2*(alt_ll-null_ll));
    return r;
}
inline double expected_shortfall_residual(
    const std::vector<double>& realized_losses,
    const std::vector<double>& var_forecasts,
    const std::vector<double>& es_forecasts){
    check_finite(realized_losses);check_finite(var_forecasts);
    check_finite(es_forecasts);
    require(realized_losses.size()==var_forecasts.size()&&
            realized_losses.size()==es_forecasts.size(),
            "ES residual series shape");
    double sum=0;int count=0;
    for(int i=0;i<int(realized_losses.size());i++)
        if(realized_losses[i]>var_forecasts[i]){
            sum+=realized_losses[i]-es_forecasts[i];count++;
        }
    require(count>0,"No VaR exceptions for ES residual");
    return sum/count;
}
} // namespace kline
