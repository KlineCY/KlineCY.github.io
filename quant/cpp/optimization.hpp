#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

inline std::vector<double> project_box(const std::vector<double>& values,
                                       const std::vector<double>& lower,
                                       const std::vector<double>& upper) {
    check_finite(values); check_finite(lower); check_finite(upper);
    require(values.size()==lower.size() && values.size()==upper.size(),
            "Box projection dimensions differ");
    std::vector<double> out(values.size());
    for (int i=0;i<int(values.size());i++) {
        require(lower[i]<=upper[i],"Invalid box bounds");
        out[i]=std::max(lower[i],std::min(values[i],upper[i]));
    }
    return out;
}

inline std::vector<double> project_l1_ball(const std::vector<double>& values,
                                            double radius) {
    check_finite(values);
    require(std::isfinite(radius) && radius>=0,"Invalid L1 radius");
    double norm=0;
    for(double value:values)norm+=std::fabs(value);
    if(norm<=radius)return values;
    if(radius==0)return std::vector<double>(values.size(),0);
    std::vector<double> absolute=values;
    for(double& v:absolute)v=std::fabs(v);
    std::sort(absolute.begin(),absolute.end(),std::greater<double>());
    double sum=0,threshold=0;
    for(int i=0;i<int(absolute.size());i++){
        sum+=absolute[i];
        double candidate=(sum-radius)/(i+1);
        if(absolute[i]>candidate)threshold=candidate;
    }
    std::vector<double> out(values.size());
    for(int i=0;i<int(values.size());i++)
        out[i]=std::copysign(std::max(0.0,std::fabs(values[i])-threshold),values[i]);
    return out;
}

inline double quadratic_objective(const Matrix& h,const std::vector<double>& linear,
                                  const std::vector<double>& x) {
    auto hx=covariance_times_weights(h,x);
    check_finite(linear);
    require(x.size()==linear.size(),"Quadratic dimensions differ");
    return 0.5*std::inner_product(x.begin(),x.end(),hx.begin(),0.0)-
           std::inner_product(x.begin(),x.end(),linear.begin(),0.0);
}

inline std::vector<double> solve_box_quadratic(
    const Matrix& h,const std::vector<double>& linear,
    const std::vector<double>& lower,const std::vector<double>& upper,
    int iterations=8000) {
    check_covariance(h);check_finite(linear);
    require(h.size()==linear.size() && h.size()==lower.size() &&
            h.size()==upper.size() && iterations>0,"Invalid box QP dimensions");
    std::vector<double> x=project_box(std::vector<double>(linear.size(),0),lower,upper);
    double bound=0;
    for(const auto& row:h){
        double sum=0;for(double v:row)sum+=std::fabs(v);
        bound=std::max(bound,sum);
    }
    require(bound>0,"Box QP requires nonzero Hessian");
    double step=1.0/bound;
    for(int iter=0;iter<iterations;iter++){
        auto hx=covariance_times_weights(h,x);
        std::vector<double> next(x.size());
        double change=0;
        for(int i=0;i<int(x.size());i++){
            next[i]=std::max(lower[i],std::min(upper[i],
                             x[i]-step*(hx[i]-linear[i])));
            change=std::max(change,std::fabs(next[i]-x[i]));
        }
        x=next;
        if(change<1e-11)break;
    }
    return x;
}

inline double finite_difference_derivative(
    const std::vector<double>& samples,double spacing,int at) {
    check_finite(samples);
    require(samples.size()>=3 && std::isfinite(spacing) && spacing>0 &&
            at>0 && at+1<int(samples.size()),"Central difference needs neighbors");
    return (samples[at+1]-samples[at-1])/(2*spacing);
}

} // namespace kline
