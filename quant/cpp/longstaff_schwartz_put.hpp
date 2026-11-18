#pragma once
#include "householder_qr.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double longstaff_schwartz_american_put(
    const Matrix& simulated_spot_paths,double strike,double rate,double dt){
    require(simulated_spot_paths.size()>=4&&
            std::isfinite(strike)&&strike>0&&std::isfinite(rate)&&
            std::isfinite(dt)&&dt>0,"Invalid LSM paths or contract");
    int n=int(simulated_spot_paths.size());
    int times=int(simulated_spot_paths[0].size());
    require(times>=3,"LSM requires multiple exercise dates");
    for(const auto& path:simulated_spot_paths){
        check_finite(path);require(int(path.size())==times,
              "LSM mismatched path length");
        for(double price:path)require(price>0,"LSM requires positive spots");
    }
    double discount=std::exp(-rate*dt);
    std::vector<double> cashflows(n);
    for(int i=0;i<n;i++)
        cashflows[i]=std::max(strike-simulated_spot_paths[i].back(),0.0);
    for(int t=times-2;t>=1;t--){
        for(double& payoff:cashflows)payoff*=discount;
        Matrix x;std::vector<double> y;std::vector<int> in_money;
        for(int i=0;i<n;i++){
            double spot=simulated_spot_paths[i][t];
            if(spot<strike){
                x.push_back({spot,spot*spot});
                y.push_back(cashflows[i]);in_money.push_back(i);
            }
        }
        if(int(x.size())>=5){
            try{
                auto regression=householder_ols_intercept(x,y);
                for(int k=0;k<int(in_money.size());k++){
                    double spot=simulated_spot_paths[in_money[k]][t];
                    double continuation=regression.coefficients[0]+
                        regression.coefficients[1]*spot+
                        regression.coefficients[2]*spot*spot;
                    double exercise=strike-spot;
                    if(exercise>continuation)cashflows[in_money[k]]=exercise;
                }
            }catch(const std::invalid_argument&){
                double continuation=mean(y);
                for(int index:in_money)
                    if(strike-simulated_spot_paths[index][t]>continuation)
                        cashflows[index]=strike-simulated_spot_paths[index][t];
            }
        }
    }
    double result=0;
    for(double payoff:cashflows)result+=discount*payoff/n;
    double immediate=0;
    for(const auto& path:simulated_spot_paths)
        immediate+=std::max(strike-path.front(),0.0)/n;
    return std::max(result,immediate);
}
} // namespace kline
