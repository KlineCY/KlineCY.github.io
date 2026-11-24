#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_volatility {
using namespace evidence;
inline Report forward_variance(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);Report o{{"Start","End","TotalVarianceStart","TotalVarianceEnd","ForwardVariance","ForwardVol"},{}};
    for(auto&r:x){require(r[0]>=0&&r[2]>=0&&r[3]>=0,"Invalid tenor or vol");double h=positive(r[1]-r[0]),a=r[0]*r[2]*r[2],b=r[1]*r[3]*r[3];require(b-a>=-1e-12*std::max({1.,a,b}),"Negative forward variance");double v=std::max(0.,(b-a)/h);o.rows.push_back({r[0],r[1],a,b,v,std::sqrt(v)});}return o;
}

inline Report noise_signature(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);require(p[0]>=0&&p[1]>=0,"Negative variance");Report o{{"Intervals","EfficientVariance","NoiseBias","ExpectedRV"},{}};
    for(auto&r:x){auto n=count(r[0],1,1000000000);double v=2*n*p[1];o.rows.push_back({double(n),p[0],v,p[0]+v});}return o;
}

inline Report lognormal_vol_convexity(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);Report o{{"MeanVariance","SqrtMeanVariance","ExpectedVol","ConvexityGap"},{}};
    for(auto&r:x){double a=std::sqrt(positive(r[0]));require(r[1]>=0,"Negative log variance");double b=a*std::exp(-r[1]/8);o.rows.push_back({r[0],a,b,a-b});}return o;
}

inline Report gjr_news_impact(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,5);double w=positive(p[0]),a=p[1],g=p[2],b=p[3],h=positive(p[4]);require(a>=0&&g>=0&&b>=0,"Negative GJR parameter");Report o{{"Innovation","SymmetricComponent","AsymmetryComponent","NextVariance"},{}};
    for(auto&r:x){double e=r[0],q=e*e,u=a*q,v=e<0?g*q:0;o.rows.push_back({e,u,v,w+u+v+b*h});}return o;
}
} }
