#pragma once
#include "stats.hpp"
#include <cmath>
#include <functional>
namespace kline {
inline double bisect_root(const std::function<double(double)>& f,
                          double left,double right,double tolerance=1e-12){
    require(std::isfinite(left)&&std::isfinite(right)&&left<right&&
            std::isfinite(tolerance)&&tolerance>0,"Invalid root bracket");
    double fl=f(left),fr=f(right);
    require(std::isfinite(fl)&&std::isfinite(fr)&&fl*fr<=0,"Root not bracketed");
    if(fl==0)return left;if(fr==0)return right;
    for(int i=0;i<200;i++){
        double mid=left+0.5*(right-left),fm=f(mid);
        require(std::isfinite(fm),"Nonfinite bracket objective");
        if(fm==0||right-left<=tolerance)return mid;
        if((fm>0)==(fl>0)){left=mid;fl=fm;}else right=mid;
    }
    throw std::runtime_error("Bisection did not converge");
}
inline double safeguarded_newton(const std::function<double(double)>& f,
             const std::function<double(double)>& derivative,
             double left,double right,double tolerance=1e-11){
    require(std::isfinite(left)&&std::isfinite(right)&&left<right&&
            tolerance>0&&std::isfinite(tolerance),"Invalid Newton bracket");
    double fl=f(left),fr=f(right);
    require(std::isfinite(fl)&&std::isfinite(fr)&&fl*fr<=0,
            "Newton requires a sign-changing bracket");
    double x=(left+right)*0.5;
    for(int i=0;i<150;i++){
        double fx=f(x);
        require(std::isfinite(fx),"Nonfinite Newton function");
        if(std::fabs(fx)<tolerance||right-left<tolerance)return x;
        if((fx>0)==(fl>0)){left=x;fl=fx;}else right=x;
        double slope=derivative(x),candidate=(left+right)*0.5;
        if(std::isfinite(slope)&&slope!=0){
            double step=x-fx/slope;
            if(step>left&&step<right)candidate=step;
        }
        x=candidate;
    }
    throw std::runtime_error("Safeguarded Newton did not converge");
}
} // namespace kline
