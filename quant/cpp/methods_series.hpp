#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_series {
using namespace evidence;
inline Report ma_autocov(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,2);
    double t=p[0],v=positive(p[1]),g=(1+t*t)*v;
    Report o{{"Lag","Autocovariance","Autocorrelation"},{}};
    for(auto&r:x){auto k=count(r[0],0,1000000);
    double a=k==0?g:k==1?t*v:0;
    o.rows.push_back({double(k),a,a/g});
    }return o;
    
}

inline Report harmonic_power(const Mat& x,const Vec& p){
    validate(x,2,1,1);
    parameters(p,1);
    auto n=x.size(),k=count(p[0],0,n-1);
    double a=0,b=0,s=0;
    for(std::size_t i=0;
    i<n;
    ++i){double w=2*std::acos(-1.)*k*i/n,z=x[i][0];
    a+=z*std::cos(w);
    b-=z*std::sin(w);
    s+=z*z;
    }return scalar({"Real","Imaginary","Periodogram","MeanSquare"},{a,b,(a*a+b*b)/n,s/n});
    
}

inline Report overlap_mean(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,1);
    double v=positive(p[0]);
    Report o{{"Horizon","Variance","LagOneCovariance","LagOneCorrelation"},{}};
    for(auto&r:x){auto h=count(r[0],1,1000000);
    o.rows.push_back({double(h),h*v,(h-1)*v,double(h-1)/h});
    }return o;
    
}

inline Report seasonal_difference(const Mat& x,const Vec& p){
    validate(x,2,1,1);
    parameters(p,1);
    auto k=count(p[0],1,x.size()-1);
    Report o{{"Time","Current","Lagged","Difference"},{}};
    for(std::size_t i=k;
    i<x.size();
    ++i)o.rows.push_back({double(i+1),x[i][0],x[i-k][0],x[i][0]-x[i-k][0]});
    return o;
    
}
} }
