#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_temporal {
using namespace evidence;
inline Report ar_impulse(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);double a=p[0];require(std::abs(a)<1,"Stationary AR required");Report o{{"Horizon","Impulse","Cumulative"},{}};
    for(auto&r:x){auto h=count(r[0],0,1000000);double z=std::pow(a,double(h));o.rows.push_back({double(h),z,(1-std::pow(a,double(h+1)))/(1-a)});}return o;
}

inline Report ar_forecast_variance(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);double a=p[0],v=positive(p[1]);require(std::abs(a)<1,"Stationary AR required");double z=a*a,L=v/(1-z);Report o{{"Horizon","ForecastErrorVariance","LongRunVariance"},{}};
    for(auto&r:x){auto h=count(r[0],1,1000000);double q=z==0?1:-std::expm1(h*std::log(z));o.rows.push_back({double(h),L*q,L});}return o;
}

inline Report filter_frequency(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);double w=p[0];require(w>=0&&w<=std::acos(-1.),"Frequency outside Nyquist interval");double a=0,b=0;
    for(std::size_t j=0;j<x.size();++j){a+=x[j][0]*std::cos(w*j);b-=x[j][0]*std::sin(w*j);}
    double v=std::hypot(a,b);return scalar({"Frequency","Real","Imag","Gain","GainSquared"},{w,a,b,v,v*v});
}

inline Report lag_aligned_cov(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,1);auto h=count(p[0],0,x.size()-2),n=x.size()-h;double a=0,b=0;
    for(std::size_t i=0;i<n;++i){a+=x[i][0]/n;b+=x[i+h][1]/n;}double c=0;
    for(std::size_t i=0;i<n;++i)c+=(x[i][0]-a)*(x[i+h][1]-b)/n;
    return scalar({"Lag","Pairs","MeanX","MeanY","Covariance"},{double(h),double(n),a,b,c});
}
} }
