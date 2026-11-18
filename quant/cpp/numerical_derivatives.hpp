#pragma once
#include "stats.hpp"
#include <cmath>
#include <functional>
#include <vector>
namespace kline {
inline std::vector<double> finite_difference_gradient(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& x,double step=1e-5){
    check_finite(x);
    require(std::isfinite(step)&&step>0,"Invalid gradient step");
    std::vector<double> grad(x.size());
    for(int i=0;i<int(x.size());i++){
        auto plus=x,minus=x;
        double h=step*std::max(1.0,std::fabs(x[i]));
        plus[i]+=h;minus[i]-=h;
        double fp=f(plus),fm=f(minus);
        require(std::isfinite(fp)&&std::isfinite(fm),"Nonfinite gradient value");
        grad[i]=(fp-fm)/(2*h);
    }
    return grad;
}
inline Matrix finite_difference_hessian(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& x,double step=1e-4){
    check_finite(x);
    require(std::isfinite(step)&&step>0,"Invalid Hessian step");
    int n=int(x.size());
    Matrix hessian(n,std::vector<double>(n,0));
    double center=f(x);require(std::isfinite(center),"Nonfinite Hessian center");
    for(int i=0;i<n;i++){
        double hi=step*std::max(1.0,std::fabs(x[i]));
        auto plus=x,minus=x;
        plus[i]+=hi;minus[i]-=hi;
        hessian[i][i]=(f(plus)-2*center+f(minus))/(hi*hi);
        for(int j=0;j<i;j++){
            double hj=step*std::max(1.0,std::fabs(x[j]));
            auto a=x,b=x,c=x,d=x;
            a[i]+=hi;a[j]+=hj;b[i]+=hi;b[j]-=hj;
            c[i]-=hi;c[j]+=hj;d[i]-=hi;d[j]-=hj;
            hessian[i][j]=hessian[j][i]=(f(a)-f(b)-f(c)+f(d))/(4*hi*hj);
        }
    }
    for(const auto& row:hessian)check_finite(row);
    return hessian;
}
} // namespace kline
