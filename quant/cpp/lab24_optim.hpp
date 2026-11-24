#pragma once
#include "lab24_common.hpp"
#include <functional>

namespace kline { namespace lab24 {
inline Vec simplex_projection(const Mat& x,const Vec& p){
    inputs(x,p,1,0);Vec z=col(x,0),u=z;
    std::sort(u.begin(),u.end(),std::greater<double>());
    double s=0,theta=0;std::size_t active=0;
    for(std::size_t j=0;j<u.size();++j){s+=u[j];double t=(s-1)/double(j+1);if(u[j]>t){theta=t;active=j+1;}}
    require(active>0,"Numerically empty simplex support");
    for(double& v:z)v=std::max(0.0,v-theta);
    require(std::abs(sum(z)-1)<1e-8,"Simplex projection lost precision");
    return checked(z);
}

inline Vec l1_ball(const Mat& x,const Vec& p){
    inputs(x,p,1,1);require(p[0]>=0,"Gross budget must be nonnegative");
    Vec z=col(x,0),u;for(double v:z)u.push_back(std::abs(v));
    if(sum(u)<=p[0])return z;
    if(p[0]==0)return Vec(z.size(),0);
    std::sort(u.begin(),u.end(),std::greater<double>());
    double s=0,t=0;
    for(std::size_t j=0;j<u.size();++j){s+=u[j];double q=(s-p[0])/double(j+1);if(u[j]>q)t=q;}
    for(double& v:z)v=std::copysign(std::max(0.0,std::abs(v)-t),v);
    double gross=0;for(double v:z)gross+=std::abs(v);
    require(std::abs(gross-p[0])<=1e-8*std::max(1.0,p[0]),"L1 projection lost precision");
    return checked(z);
}

inline Vec box_budget(const Mat& x,const Vec& p){
    inputs(x,p,3,1);double lo=std::numeric_limits<double>::infinity(),hi=-lo,L=0,U=0;
    for(const auto& r:x){require(r[1]<=r[2],"Reversed box bounds");L+=r[1];U+=r[2];lo=std::min(lo,r[0]-r[2]);hi=std::max(hi,r[0]-r[1]);}
    require(p[0]>=L-1e-12&&p[0]<=U+1e-12,"Infeasible budget");
    Vec w(x.size());
    for(int k=0;k<100;++k){double mid=lo+(hi-lo)/2,s=0;for(const auto& r:x)s+=std::clamp(r[0]-mid,r[1],r[2]);if(s>p[0])lo=mid;else hi=mid;}
    for(std::size_t i=0;i<x.size();++i)w[i]=std::clamp(x[i][0]-(lo+(hi-lo)/2),x[i][1],x[i][2]);
    require(std::abs(sum(w)-p[0])<=1e-8*std::max(1.0,std::abs(p[0])),"Budget residual too large");
    return checked(w);
}

inline Vec elastic_coordinate(const Mat& x,const Vec& p){
    inputs(x,p,2,2);require(p[0]>=0&&p[1]>=0,"Penalty must be nonnegative");
    Vec w;
    for(const auto& r:x){positive(r[1]);w.push_back(std::copysign(std::max(0.0,std::abs(r[0])-p[0]),r[0])/(r[1]+p[1]));}
    return checked(w);
}

inline Vec entropy_tilt(const Mat& x,const Vec& p){
    inputs(x,p,2,1);Vec logw;double total=0;
    for(const auto& r:x){positive(r[0]);total+=r[0];logw.push_back(std::log(r[0])+p[0]*r[1]);}
    require(std::abs(total-1)<1e-10,"Prior probabilities must sum to one");
    check_finite(logw);double m=*std::max_element(logw.begin(),logw.end()),z=0;
    for(double q:logw)z+=std::exp(q-m);
    double expectation=0,kl=0;
    for(std::size_t i=0;i<x.size();++i){double q=std::exp(logw[i]-m)/z;expectation+=q*x[i][1];if(q>0)kl+=q*(std::log(q)-std::log(x[i][0]));}
    return checked({expectation,kl});
}
} }
