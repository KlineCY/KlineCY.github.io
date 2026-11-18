#pragma once
#include "ols_models.hpp"
#include <cmath>
#include <vector>

namespace kline {
struct CointegrationRegression {
    double intercept=0;
    double hedge_ratio=0;
    std::vector<double> spread;
    double residual_ar_slope=0;
    double residual_half_life=0;
};

inline CointegrationRegression two_asset_spread_regression(
    const std::vector<double>& dependent,
    const std::vector<double>& hedge) {
    check_finite(dependent);check_finite(hedge);
    require(dependent.size()==hedge.size()&&dependent.size()>=6,
            "Cointegration diagnostic needs at least six paired levels");
    Matrix design;
    for(double x:hedge)design.push_back({x});
    auto model=ordinary_least_squares(design,dependent);
    CointegrationRegression result;
    result.intercept=model.coefficients[0];
    result.hedge_ratio=model.coefficients[1];
    result.spread=model.residuals;
    std::vector<double> lag(result.spread.begin(),result.spread.end()-1);
    std::vector<double> next(result.spread.begin()+1,result.spread.end());
    if(variance(lag,false)==0)return result;
    result.residual_ar_slope=covariance(lag,next,false)/variance(lag,false);
    if(result.residual_ar_slope>0 && result.residual_ar_slope<1)
        result.residual_half_life=std::log(0.5)/std::log(result.residual_ar_slope);
    return result;
}

inline double unit_root_dickey_fuller_t(const std::vector<double>& levels) {
    check_finite(levels);
    require(levels.size()>=6,"Unit root diagnostic needs six levels");
    Matrix x;
    std::vector<double> changes;
    for(int t=1;t<int(levels.size());t++){
        x.push_back({levels[t-1]});
        changes.push_back(levels[t]-levels[t-1]);
    }
    auto fit=ordinary_least_squares(x,changes);
    double center=mean(levels);
    double sum=0;
    for(int t=0;t<int(x.size());t++)sum+=std::pow(x[t][0]-center,2);
    require(sum>0 && fit.mse>0,"Dickey-Fuller t requires stochastic variability");
    return fit.coefficients[1]/std::sqrt(fit.mse/sum);
}

inline std::vector<double> spread_zscores(const std::vector<double>& spread,
                                           int lookback) {
    check_finite(spread);
    require(lookback>=2 && lookback<int(spread.size()),
            "Spread signal needs a historical window");
    std::vector<double> values;
    for(int t=lookback;t<int(spread.size());t++){
        std::vector<double> past(spread.begin()+t-lookback,spread.begin()+t);
        double sigma=std::sqrt(variance(past,false));
        values.push_back(sigma>0?(spread[t]-mean(past))/sigma:0);
    }
    return values;
}
} // namespace kline
