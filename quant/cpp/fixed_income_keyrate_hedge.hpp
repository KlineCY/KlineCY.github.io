#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> solve_keyrate_dv01_hedge(
    const Matrix& hedge_instruments_by_keyrate,
    const std::vector<double>& portfolio_keyrate_dv01){
    check_finite(portfolio_keyrate_dv01);
    int n=int(portfolio_keyrate_dv01.size());
    require(int(hedge_instruments_by_keyrate.size())==n&&n>0,
            "Key rate exposures must have as many instruments as nodes");
    Matrix by_rate(n,std::vector<double>(n));
    for(int j=0;j<n;j++){
        check_finite(hedge_instruments_by_keyrate[j]);
        require(int(hedge_instruments_by_keyrate[j].size())==n,
                "Hedge instrument dimensions");
        for(int i=0;i<n;i++)by_rate[i][j]=hedge_instruments_by_keyrate[j][i];
    }
    std::vector<double> target(n);
    for(int i=0;i<n;i++)target[i]=-portfolio_keyrate_dv01[i];
    return solve_linear_system(by_rate,target);
}
} // namespace kline
