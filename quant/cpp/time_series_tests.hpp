#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double ljung_box_q(const std::vector<double>& x,int lags){
    check_finite(x);int n=int(x.size());
    require(n>=4&&lags>=1&&lags<n-1,"Invalid Ljung-Box lags");
    double center=mean(x),den=0;
    for(double v:x)den+=(v-center)*(v-center);
    require(den>0,"Constant series has undefined Ljung Box");
    double sum=0;
    for(int k=1;k<=lags;k++){
        double numerator=0;
        for(int t=k;t<n;t++)numerator+=(x[t]-center)*(x[t-k]-center);
        double rho=numerator/den;
        sum+=rho*rho/(n-k);
    }
    return n*(n+2.0)*sum;
}
inline double jarque_bera_statistic(const std::vector<double>& x){
    check_finite(x);
    require(x.size()>=4,"Jarque-Bera requires four samples");
    double avg=mean(x),second=0,third=0,fourth=0;
    for(double v:x){
        double d=v-avg;second+=d*d;third+=d*d*d;fourth+=d*d*d*d;
    }
    second/=x.size();third/=x.size();fourth/=x.size();
    require(second>0,"Jarque-Bera zero variation");
    double skew=third/std::pow(second,1.5);
    double kurt=fourth/(second*second);
    return x.size()*(skew*skew/6+std::pow(kurt-3,2)/24);
}
inline double engle_arch_lm(const std::vector<double>& residuals,int lags){
    check_finite(residuals);
    require(lags>=1&&int(residuals.size())>=3*lags+5,
            "ARCH LM requires more residual observations");
    int n=int(residuals.size());
    Matrix x;std::vector<double> y;
    for(int t=lags;t<n;t++){
        std::vector<double> row;for(int j=1;j<=lags;j++)
            row.push_back(residuals[t-j]*residuals[t-j]);
        x.push_back(row);y.push_back(residuals[t]*residuals[t]);
    }
    auto fit=ordinary_least_squares(x,y);
    return y.size()*std::max(0.0,fit.r_squared);
}
} // namespace kline
