#pragma once
#include "stats.hpp"
#include <cmath>
#include <complex>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<std::complex<double>> discrete_fourier_transform(
    const std::vector<double>& series){
    check_finite(series);int n=int(series.size());
    std::vector<std::complex<double>> result(n);
    const double pi=std::acos(-1.0);
    for(int k=0;k<n;k++)
        for(int t=0;t<n;t++){
            double phase=-2*pi*k*t/n;
            result[k]+=series[t]*std::complex<double>(std::cos(phase),std::sin(phase));
        }
    return result;
}
inline std::vector<double> periodogram(const std::vector<double>& series,
                                       bool center=true){
    check_finite(series);
    std::vector<double> x=series;
    if(center){double m=mean(x);for(double& v:x)v-=m;}
    auto fft=discrete_fourier_transform(x);
    std::vector<double> power;
    for(int k=0;k<=int(x.size())/2;k++)
        power.push_back(std::norm(fft[k])/x.size());
    return power;
}
inline double spectral_entropy(const std::vector<double>& power){
    check_finite(power);double total=0;
    for(double v:power){require(v>=0,"Negative spectral power");total+=v;}
    require(total>0&&power.size()>1,"No spectral variation");
    double h=0;
    for(double v:power)if(v>0){
        double p=v/total;h-=p*std::log(p);
    }
    return h/std::log(double(power.size()));
}
inline double dominant_frequency(const std::vector<double>& x){
    require(x.size()>=4,"Frequency requires four data points");
    auto power=periodogram(x);
    int k=1;
    for(int j=2;j<int(power.size());j++)
        if(power[j]>power[k])k=j;
    return double(k)/x.size();
}
} // namespace kline
