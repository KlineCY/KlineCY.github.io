#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct GaussianMixtureTwo{
    double weight=0.5,mean0=0,mean1=0,var0=1,var1=1;
    double log_likelihood=0;
};
inline GaussianMixtureTwo fit_gaussian_mixture_two(
    const std::vector<double>& observations,int iterations=150,
    double variance_floor=1e-6){
    check_finite(observations);
    require(observations.size()>=6&&iterations>0&&iterations<=10000&&
            std::isfinite(variance_floor)&&variance_floor>0,
            "Invalid two Gaussian mixture inputs");
    GaussianMixtureTwo m;
    m.mean0=quantile(observations,0.25);
    m.mean1=quantile(observations,0.75);
    double overall=std::max(variance_floor,variance(observations,false));
    m.var0=m.var1=overall;
    for(int iteration=0;iteration<iterations;iteration++){
        double n0=0,n1=0,s0=0,s1=0,v0=0,v1=0,loss=0;
        std::vector<double> responsibilities;
        for(double x:observations){
            double a=m.weight*std::exp(-std::pow(x-m.mean0,2)/(2*m.var0))/
                     std::sqrt(2*std::acos(-1.0)*m.var0);
            double b=(1-m.weight)*std::exp(-std::pow(x-m.mean1,2)/(2*m.var1))/
                     std::sqrt(2*std::acos(-1.0)*m.var1);
            double z=a+b;
            require(z>0&&std::isfinite(z),"Gaussian mixture likelihood underflow");
            double p=a/z;
            responsibilities.push_back(p);
            n0+=p;n1+=1-p;s0+=p*x;s1+=(1-p)*x;
            loss+=std::log(z);
        }
        require(n0>1e-9&&n1>1e-9,"Degenerate Gaussian mixture component");
        double mu0=s0/n0,mu1=s1/n1;
        for(int i=0;i<int(observations.size());i++){
            v0+=responsibilities[i]*std::pow(observations[i]-mu0,2);
            v1+=(1-responsibilities[i])*std::pow(observations[i]-mu1,2);
        }
        m.weight=std::max(1e-6,std::min(1-1e-6,n0/observations.size()));
        m.mean0=mu0;m.mean1=mu1;
        m.var0=std::max(variance_floor,v0/n0);
        m.var1=std::max(variance_floor,v1/n1);
        m.log_likelihood=loss;
    }
    return m;
}
} // namespace kline
