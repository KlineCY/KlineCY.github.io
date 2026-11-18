#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct NaturalSpline {
    std::vector<double> x,a,b,c,d;
};
inline NaturalSpline fit_natural_spline(const std::vector<double>& knots,
                                       const std::vector<double>& values){
    check_finite(knots);check_finite(values);
    require(knots.size()==values.size()&&knots.size()>=2,
            "Spline requires ordered knots and values");
    int n=int(knots.size())-1;
    std::vector<double> h(n);
    for(int i=0;i<n;i++){
        h[i]=knots[i+1]-knots[i];require(h[i]>0,"Spline knots not increasing");
    }
    std::vector<double> rhs(n+1,0),diagonal(n+1,1),upper(n+1,0),
                        lower(n+1,0),second(n+1,0);
    for(int i=1;i<n;i++){
        lower[i]=h[i-1];diagonal[i]=2*(h[i-1]+h[i]);upper[i]=h[i];
        rhs[i]=3*((values[i+1]-values[i])/h[i]-
                  (values[i]-values[i-1])/h[i-1]);
    }
    for(int i=1;i<=n;i++){
        double factor=lower[i]/diagonal[i-1];
        diagonal[i]-=factor*upper[i-1];
        rhs[i]-=factor*rhs[i-1];
    }
    for(int i=n-1;i>=0;i--)
        second[i]=(rhs[i]-upper[i]*second[i+1])/diagonal[i];
    NaturalSpline result;result.x=knots;result.a=values;
    for(int i=0;i<n;i++){
        result.b.push_back((values[i+1]-values[i])/h[i]-
                           h[i]*(2*second[i]+second[i+1])/3);
        result.c.push_back(second[i]);
        result.d.push_back((second[i+1]-second[i])/(3*h[i]));
    }
    return result;
}
inline double spline_value(const NaturalSpline& spline,double x){
    require(std::isfinite(x)&&spline.x.size()>=2,"Invalid spline query");
    require(x>=spline.x.front()&&x<=spline.x.back(),"Spline extrapolation unsupported");
    auto it=std::upper_bound(spline.x.begin(),spline.x.end(),x);
    int i=std::max(0,std::min(int(spline.b.size())-1,int(it-spline.x.begin())-1));
    double dx=x-spline.x[i];
    return spline.a[i]+dx*(spline.b[i]+dx*(spline.c[i]+dx*spline.d[i]));
}
} // namespace kline
