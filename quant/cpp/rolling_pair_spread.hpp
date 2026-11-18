#pragma once
#include "householder_qr.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct RollingSpreadPoint{double intercept=0,hedge_ratio=0,spread=0;};
inline std::vector<RollingSpreadPoint> past_only_pair_spreads(
    const std::vector<double>& dependent,const std::vector<double>& hedge,
    int lookback){
    check_finite(dependent);check_finite(hedge);
    require(dependent.size()==hedge.size()&&
            lookback>=4&&lookback<int(dependent.size()),
            "Invalid rolling cointegration data window");
    std::vector<RollingSpreadPoint> output;
    for(int t=lookback;t<int(dependent.size());t++){
        Matrix x;std::vector<double> y;
        for(int i=t-lookback;i<t;i++){
            x.push_back({hedge[i]});y.push_back(dependent[i]);
        }
        auto fit=householder_ols_intercept(x,y);
        output.push_back({fit.coefficients[0],fit.coefficients[1],
             dependent[t]-fit.coefficients[0]-fit.coefficients[1]*hedge[t]});
    }
    return output;
}
} // namespace kline
