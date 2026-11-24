#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_optiongreeks {
using namespace evidence;
inline Report digital_decomposition(const Mat& x,const Vec& p){
    validate(x,1,6,6);parameters(p,0);Report o{{"CashDigital","AssetDigital","ReplicatedCall","RiskNeutralITM"},{}};
    for(auto&r:x){double s=positive(r[0]),k=positive(r[1]),v=positive(r[4]),t=positive(r[5]),z=v*std::sqrt(t),d1=(std::log(s/k)+(r[2]-r[3]+.5*v*v)*t)/z,d2=d1-z;double pr=normal(d2),a=std::exp(-r[2]*t)*pr,b=s*std::exp(-r[3]*t)*normal(d1);o.rows.push_back({a,b,b-k*a,pr});}return o;
}

inline Report vanna_volga(const Mat& x,const Vec& p){
    validate(x,1,6,6);parameters(p,0);Report o{{"Vega","Vanna","Volga"},{}};double c=std::sqrt(2*std::acos(-1.));
    for(auto&r:x){double s=positive(r[0]),k=positive(r[1]),v=positive(r[4]),t=positive(r[5]),z=v*std::sqrt(t),d1=(std::log(s/k)+(r[2]-r[3]+.5*v*v)*t)/z,d2=d1-z,f=std::exp(-d1*d1/2)/c,df=std::exp(-r[3]*t),vega=s*df*f*std::sqrt(t);o.rows.push_back({vega,-df*f*d2/v,vega*d1*d2/v});}return o;
}

inline Report delta_hedge_variance(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);require(p[0]>=0,"Negative implied vol");double s=0;Report o{{"Period","RealizedGamma","ImpliedTheta","NetApprox","Cumulative"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double z=positive(r[0]),h=positive(r[3]),g=.5*r[1]*z*z,a=g*r[2]*r[2],b=-g*p[0]*p[0]*h;s+=a+b;o.rows.push_back({double(i+1),a,b,a+b,s});}return o;
}

inline Report spot_vol_taylor(const Mat& x,const Vec& p){
    validate(x,1,6,6);parameters(p,2);double ds=p[0],dv=p[1];Report o{{"PositionIndex","DeltaPnL","GammaPnL","VegaPnL","VannaPnL","VolgaPnL","TotalApprox"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double n=r[0],a=n*r[1]*ds,b=.5*n*r[2]*ds*ds,c=n*r[3]*dv,d=n*r[4]*ds*dv,e=.5*n*r[5]*dv*dv;o.rows.push_back({double(i+1),a,b,c,d,e,a+b+c+d+e});}return o;
}
} }
