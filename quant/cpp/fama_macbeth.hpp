#pragma once
#include "ols_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct FamaMacBethResult{
    std::vector<double> average_coefficients;
    std::vector<double> standard_errors;
    Matrix period_coefficients;
};
inline FamaMacBethResult fama_macbeth(
    const std::vector<Matrix>& periods_features,
    const std::vector<std::vector<double>>& periods_returns){
    require(periods_features.size()==periods_returns.size()&&
            periods_features.size()>=3,"Fama MacBeth needs three periods");
    FamaMacBethResult out;
    for(int t=0;t<int(periods_features.size());t++){
        auto fit=ordinary_least_squares(periods_features[t],periods_returns[t]);
        if(t>0)require(fit.coefficients.size()==out.average_coefficients.size(),
                       "Fama MacBeth changing features");
        if(t==0)out.average_coefficients.assign(fit.coefficients.size(),0);
        out.period_coefficients.push_back(fit.coefficients);
        for(int j=0;j<int(fit.coefficients.size());j++)
            out.average_coefficients[j]+=fit.coefficients[j]/periods_features.size();
    }
    for(int j=0;j<int(out.average_coefficients.size());j++){
        std::vector<double> series;
        for(const auto& row:out.period_coefficients)series.push_back(row[j]);
        out.standard_errors.push_back(std::sqrt(variance(series)/series.size()));
    }
    return out;
}
inline double fama_macbeth_t(const FamaMacBethResult& fit,int coefficient){
    require(coefficient>=0&&coefficient<int(fit.average_coefficients.size()),
            "Missing Fama MacBeth coefficient");
    double se=fit.standard_errors[coefficient];
    require(se>0,"Constant cross sectional price of risk");
    return fit.average_coefficients[coefficient]/se;
}
} // namespace kline
