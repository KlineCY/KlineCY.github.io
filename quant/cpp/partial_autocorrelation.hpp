#pragma once
#include "cholesky_numerics.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> autocorrelations(const std::vector<double>& x,
                                            int lags){
    check_finite(x);
    require(lags>=1&&lags<int(x.size())-1,"Invalid ACF lags");
    double mu=mean(x),den=0;
    for(double v:x)den+=(v-mu)*(v-mu);
    require(den>0,"ACF undefined for constant series");
    std::vector<double> result(lags+1,1);
    for(int k=1;k<=lags;k++){
        double numerator=0;
        for(int t=k;t<int(x.size());t++)
            numerator+=(x[t]-mu)*(x[t-k]-mu);
        result[k]=numerator/den;
    }
    return result;
}
inline std::vector<double> partial_autocorrelations(
    const std::vector<double>& x,int lags){
    auto acf=autocorrelations(x,lags);
    std::vector<double> pacf(lags+1,1),previous;
    double current_error=1;
    for(int k=1;k<=lags;k++){
        double correction=0;
        for(int j=1;j<k;j++)correction+=previous[j-1]*acf[k-j];
        require(current_error>1e-12,"PACF innovation variance degenerate");
        double reflection=(acf[k]-correction)/current_error;
        pacf[k]=reflection;
        std::vector<double> next(k);
        for(int j=1;j<k;j++)
            next[j-1]=previous[j-1]-reflection*previous[k-j-1];
        next[k-1]=reflection;previous=next;
        current_error*=1-reflection*reflection;
    }
    return pacf;
}
} // namespace kline
