#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec factor_neutral_two(const Mat& x,const Vec& p){
    inputs(x,p,2,2,2);Vec z=col(x,0),f=col(x,1);
    double n=double(x.size()),sf=sum(f),ff=dot(f,f),d=positive(n*ff-sf*sf);
    double e=sum(z)-p[0],g=dot(z,f)-p[1],a=(ff*e-sf*g)/d,b=(n*g-sf*e)/d;
    for(std::size_t i=0;i<z.size();++i)z[i]-=a+b*f[i];
    return checked(z);
}

inline Vec risk_budget_diag(const Mat& x,const Vec& p){
    inputs(x,p,2,0);double b=0;Vec w;
    for(const auto& r:x){positive(r[0]);require(r[1]>=0,"Negative risk budget");b+=r[1];w.push_back(std::sqrt(r[1])/r[0]);}
    require(std::abs(b-1)<1e-10,"Risk budgets must sum to one");double z=positive(sum(w));
    for(double& q:w){q/=z;}
    return checked(w);
}

inline Vec factor_cov_risk(const Mat& x,const Vec& p){
    inputs(x,p,3,1);require(p[0]>=0,"Negative factor variance");double beta=0,idio=0;
    for(const auto& r:x){require(r[2]>=0,"Negative idiosyncratic variance");beta+=r[0]*r[1];idio+=r[0]*r[0]*r[2];}
    double sys=p[0]*beta*beta;return checked({beta,sys,idio,std::sqrt(sys+idio)});
}

inline Vec turnover_prox(const Mat& x,const Vec& p){
    inputs(x,p,4,0);Vec w;
    for(const auto& r:x){positive(r[2]);require(r[3]>=0,"Negative trading cost");double d=r[1]-r[0];w.push_back(r[0]+std::copysign(std::max(0.0,std::abs(d)-r[3]/r[2]),d));}
    return checked(w);
}

inline Vec rebalance_drift(const Mat& x,const Vec& p){
    inputs(x,p,3,0);double old=0,target=0,ret=0;
    for(const auto& r:x){require(r[0]>=0&&r[2]>=0&&r[1]>=-1,"Invalid long-only return or weight");old+=r[0];target+=r[2];ret+=r[0]*r[1];}
    require(std::abs(old-1)<1e-10&&std::abs(target-1)<1e-10,"Weights must sum to one");double gross=positive(1+ret),to=0;Vec v;
    for(const auto& r:x){double w=r[0]*(1+r[1])/gross;v.push_back(w);to+=std::abs(w-r[2]);}
    Vec out={ret,to/2};out.insert(out.end(),v.begin(),v.end());return checked(out);
}
} }
