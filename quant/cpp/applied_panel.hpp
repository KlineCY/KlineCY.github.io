#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_panel {
using namespace evidence;
inline Report within_between(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    std::map<double,Vec> g;
    for(auto&r:x)g[r[0]].push_back(r[1]);
    double a=mean(column(x,1)),w=0,b=0;
    for(auto&z:g){double m=mean(z.second);
    b+=z.second.size()*(m-a)*(m-a);
    for(double v:z.second)w+=(v-m)*(v-m);
    }return scalar({"TotalSS","WithinSS","BetweenSS"},{w+b,w,b});
    
}

inline Report cluster_mean(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    double n=x.size(),m=mean(column(x,1));
    std::map<double,double> g;
    for(auto&r:x)g[r[0]]+=r[1]-m;
    require(g.size()>1,"Two clusters required");
    double v=0,G=g.size();
    for(auto&z:g)v+=z.second*z.second/(n*n);
    return scalar({"Mean","Clusters","CR0SE","CR1SE"},{m,G,std::sqrt(v),std::sqrt(v*G/(G-1))});
    
}

inline Report first_stage(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,0);
    double a=mean(column(x,0)),b=mean(column(x,1)),s=0,t=0,c=0;
    for(auto&r:x){double d=r[0]-a,e=r[1]-b;
    s+=d*d;
    t+=e*e;
    c+=d*e;
    }double beta=c/positive(t),r2=c*c/(positive(s)*t);
    require(1-r2>1e-14,"Degenerate first stage");
    return scalar({"Intercept","Slope","R2","F"},{a-beta*b,beta,r2,r2/(1-r2)*(x.size()-2)});
    
}

inline Report omitted_bias(const Mat& x,const Vec& p){
    validate(x,3,3,3);
    parameters(p,0);
    double y=mean(column(x,0)),a=mean(column(x,1)),b=mean(column(x,2)),s=0,t=0,c=0,d=0,e=0;
    for(auto&r:x){double u=r[1]-a,v=r[2]-b,w=r[0]-y;
    s+=u*u;
    t+=v*v;
    c+=u*v;
    d+=u*w;
    e+=v*w;
    }double det=positive(s*t-c*c),beta=(d*t-e*c)/det,gamma=(e*s-d*c)/det,pi=c/positive(s),shortb=d/s;
    return scalar({"ShortSlope","LongSlope","ZCoefficient","AuxiliarySlope","Bias"},{shortb,beta,gamma,pi,shortb-beta});
    
}
} }
