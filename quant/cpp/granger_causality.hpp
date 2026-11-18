#pragma once
#include "ols_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct GrangerTest{double f_statistic=0;double restricted_rss=0;double full_rss=0;};
inline GrangerTest granger_predictive_test(
    const std::vector<double>& response,
    const std::vector<double>& candidate_predictor,int lags){
    check_finite(response);check_finite(candidate_predictor);
    require(response.size()==candidate_predictor.size()&&
            lags>=1&&int(response.size())>=5*lags+5,
            "Invalid Granger lag or sample size");
    Matrix restricted,full;std::vector<double> y;
    for(int t=lags;t<int(response.size());t++){
        std::vector<double> a,b;
        for(int j=1;j<=lags;j++)a.push_back(response[t-j]);
        b=a;
        for(int j=1;j<=lags;j++)b.push_back(candidate_predictor[t-j]);
        restricted.push_back(a);full.push_back(b);y.push_back(response[t]);
    }
    auto r=ordinary_least_squares(restricted,y);
    auto f=ordinary_least_squares(full,y);
    double rss_r=0,rss_f=0;
    for(double error:r.residuals)rss_r+=error*error;
    for(double error:f.residuals)rss_f+=error*error;
    int dof=int(y.size())-(2*lags+1);
    require(dof>0&&rss_f>0,"Granger denominator degrees or RSS invalid");
    return {std::max(0.0,(rss_r-rss_f)/lags/(rss_f/dof)),rss_r,rss_f};
}
} // namespace kline
