#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_creditmodels {
using namespace evidence;
inline Report forward_default(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);Report o{{"Start","End","UnconditionalPD","ConditionalPD","AverageHazard"},{}};
    for(auto&r:x){require(r[0]>=0&&r[2]>0&&r[2]<=1&&r[3]>0&&r[3]<=r[2],"Invalid survival interval");double h=positive(r[1]-r[0]),z=r[3]/r[2];o.rows.push_back({r[0],r[1],r[2]-r[3],1-z,-std::log(z)/h});}return o;
}

inline Report margin_period_risk(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);Report o{{"Scenario","InitialExposure","CloseoutExposure","ExposureIncrease"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[1]>=0,"Negative received collateral");double a=std::max(r[0]-r[1],0.),b=std::max(r[2]-r[1],0.);o.rows.push_back({double(i+1),a,b,b-a});}return o;
}

inline Report heterogeneous_credit_mixture(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);std::map<double,Vec> g;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]>=0&&r[2]>=0&&r[3]>=0&&r[3]<=1,"Invalid credit row");if(!g.count(r[0]))g[r[0]]={r[1],0,0};auto&v=g[r[0]];require(v[0]==r[1],"Inconsistent scenario probability");v[1]+=r[2]*r[3];v[2]+=r[2]*r[2]*r[3]*(1-r[3]);}
    double s=0,m=0,a=0;for(auto&z:g){auto&v=z.second;s+=v[0];m+=v[0]*v[1];a+=v[0]*v[2];}require(std::abs(s-1)<1e-10,"Scenario weights");double b=0;for(auto&z:g){auto&v=z.second;b+=v[0]*(v[1]-m)*(v[1]-m);}return scalar({"ExpectedLoss","WithinVariance","BetweenVariance","TotalVariance"},{m,a,b,a+b});
}

inline Report exponential_hazard_mle(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);double T=0,d=0;
    for(auto&r:x){T+=positive(r[0]);require(r[1]==0||r[1]==1,"Binary default required");d+=r[1];}double l=d/T,v=d==0?0:d*std::log(l)-l*T;return scalar({"Defaults","TotalExposure","Hazard","LogLikelihood","OneYearPD"},{d,T,l,v,-std::expm1(-l)});
}
} }
