#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double median(const std::vector<double>& x){
    check_finite(x);return quantile(x,0.5);
}
inline double median_absolute_deviation(const std::vector<double>& x){
    double mid=median(x);std::vector<double> d;
    for(double v:x)d.push_back(std::fabs(v-mid));
    return median(d);
}
inline double trimmed_mean(std::vector<double> x,double fraction){
    check_finite(x);require(std::isfinite(fraction)&&fraction>=0&&fraction<0.5,
                            "Invalid trim fraction");
    std::sort(x.begin(),x.end());
    int cut=int(std::floor(x.size()*fraction));
    require(2*cut<int(x.size()),"No observations after trimming");
    double sum=std::accumulate(x.begin()+cut,x.end()-cut,0.0);
    return sum/(x.size()-2*cut);
}
inline double robust_z_score(double value,const std::vector<double>& reference){
    require(std::isfinite(value),"Nonfinite robust score");
    double dev=median_absolute_deviation(reference);
    require(dev>0,"Zero median absolute deviation");
    return 0.6744897501960817*(value-median(reference))/dev;
}
inline double huber_location(const std::vector<double>& x,double threshold,
                             int iterations=500){
    check_finite(x);
    require(threshold>0&&std::isfinite(threshold)&&iterations>0,
            "Invalid Huber location parameters");
    double center=median(x);
    for(int step=0;step<iterations;step++){
        double sum=0;
        for(double v:x)sum+=std::max(-threshold,std::min(threshold,v-center));
        double next=center+sum/x.size();
        if(std::fabs(next-center)<1e-11)return next;
        center=next;
    }
    return center;
}
inline double lag_autocorrelation(const std::vector<double>& x,int lag){
    check_finite(x);
    require(lag>0&&lag<int(x.size()),"Invalid autocorrelation lag");
    double center=mean(x),num=0,den=0;
    for(int t=lag;t<int(x.size());t++)
        num+=(x[t]-center)*(x[t-lag]-center);
    for(double value:x)den+=std::pow(value-center,2);
    require(den>0,"Constant series autocorrelation undefined");
    return num/den;
}
} // namespace kline
