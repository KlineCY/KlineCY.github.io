#pragma once
#include "lab24_common.hpp"

namespace kline { namespace lab24 {
inline Vec discount_bootstrap(const Mat& x,const Vec& p){
    inputs(x,p,1,0);Vec d;double s=0;
    for(const auto& r:x){require(r[0]>-1,"Invalid coupon rate");double q=positive((1-r[0]*s)/(1+r[0]));d.push_back(q);s+=q;}
    return checked(d);
}

inline Vec forward_curve(const Mat& x,const Vec& p){
    inputs(x,p,2,0);double t=0,logd=0;Vec out;
    for(const auto& r:x){require(r[0]>t,"Maturities must increase");double q=std::log(positive(r[1]));out.push_back((logd-q)/(r[0]-t));t=r[0];logd=q;}
    return checked(out);
}

inline Vec curve_keyrate(const Mat& x,const Vec& p){
    inputs(x,p,2,4);require(p[0]>0&&p[1]>p[0],"Invalid key-rate nodes");
    double pv=0,k1=0,k2=0;
    for(const auto& r:x){require(r[0]>=0,"Negative cash-flow time");double h=std::clamp((r[0]-p[0])/(p[1]-p[0]),0.0,1.0);double v=r[1]*std::exp(-r[0]*((1-h)*p[2]+h*p[3]));pv+=v;k1+=v*(-std::expm1(-.0001*r[0]*(1-h)));k2+=v*(-std::expm1(-.0001*r[0]*h));}
    return checked({pv,k1,k2});
}

inline Vec duration_immunize(const Mat& x,const Vec& p){
    inputs(x,p,1,3,2);require(x.size()==2,"Two instruments required");
    double a=x[0][0],b=x[1][0],l=positive(p[0]),t=p[1],r=p[2];require(a>0&&b>a&&t>=a&&t<=b,"Invalid maturity bracket");
    double v2=l*(t-a)/(b-a),v1=l-v2;
    return checked({v1*std::exp(r*a),v2*std::exp(r*b),v1,v2});
}

inline Vec yield_newton(const Mat& x,const Vec& p){
    inputs(x,p,2,3);double price=positive(p[0]),lo=p[1],hi=p[2];require(lo<hi,"Reversed yield bracket");
    for(const auto& r:x){positive(r[0]);positive(r[1]);}
    auto f=[&](double y){double v=-price,d=0;for(const auto& r:x){double c=r[1]*std::exp(-y*r[0]);v+=c;d-=r[0]*c;}require(std::isfinite(v)&&std::isfinite(d),"Yield bracket causes overflow");return std::pair<double,double>{v,d};};
    require(f(lo).first>=0&&f(hi).first<=0,"Price not bracketed");
    double y=lo+(hi-lo)/2;
    for(int i=0;i<150;++i){auto a=f(y);if(std::abs(a.first)<1e-12*price)break;if(a.first>0)lo=y;else hi=y;double z=y-a.first/a.second;y=std::isfinite(z)&&z>lo&&z<hi?z:lo+(hi-lo)/2;}
    require(std::abs(f(y).first)<=1e-8*price,"Yield iteration did not converge");return checked({y,f(y).first});
}
} }
