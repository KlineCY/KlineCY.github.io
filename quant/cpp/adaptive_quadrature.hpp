#pragma once
#include "stats.hpp"
#include <cmath>
#include <functional>
namespace kline {
inline double simpson_estimate(const std::function<double(double)>& f,
                               double a,double b){
    double c=a+0.5*(b-a),fa=f(a),fb=f(b),fc=f(c);
    require(std::isfinite(fa)&&std::isfinite(fb)&&std::isfinite(fc),
            "Nonfinite integrand value");
    return (b-a)*(fa+4*fc+fb)/6;
}
inline double adaptive_simpson(const std::function<double(double)>& f,
       double a,double b,double tolerance=1e-8,int max_depth=20){
    require(std::isfinite(a)&&std::isfinite(b)&&a<b&&tolerance>0&&
            std::isfinite(tolerance)&&max_depth>=1,"Invalid integration range");
    std::function<double(double,double,double,double,int)> recurse;
    recurse=[&](double lo,double hi,double integral,double eps,int depth){
        double mid=lo+0.5*(hi-lo);
        double l=simpson_estimate(f,lo,mid),r=simpson_estimate(f,mid,hi);
        double discrepancy=l+r-integral;
        if(depth==0){
            require(std::fabs(discrepancy)<=15*eps,
                    "Adaptive Simpson exhausted maximum depth");
            return l+r+discrepancy/15;
        }
        if(std::fabs(discrepancy)<=15*eps)return l+r+discrepancy/15;
        return recurse(lo,mid,l,eps/2,depth-1)+
               recurse(mid,hi,r,eps/2,depth-1);
    };
    return recurse(a,b,simpson_estimate(f,a,b),tolerance,max_depth);
}
} // namespace kline
