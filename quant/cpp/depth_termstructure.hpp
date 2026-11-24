#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_termstructure {
using namespace evidence;
inline Report log_discount_forward(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);Report o{{"Start","End","DiscountRatio","ContinuousForward"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]>=0,"Negative time");positive(r[1]);if(i){auto&a=x[i-1];double h=positive(r[0]-a[0]);o.rows.push_back({a[0],r[0],r[1]/a[1],(std::log(a[1])-std::log(r[1]))/h});}}return o;
}

inline Report forward_swap_annuity(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,3);double start=positive(p[0]),N=positive(p[1]),K=p[2],a=0;
    for(auto&r:x)a+=positive(r[0])*positive(r[1]);positive(a);
    double f=N*(start-x.back()[1]),v=N*K*a;return scalar({"Annuity","ParRate","FixedPV","FloatPV","ReceiveFixedPV","DV01"},{a,(start-x.back()[1])/a,v,f,v-f,N*a*1e-4});
}

inline Report fra_settlement(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);double N=positive(p[0]),K=p[1];Report o{{"FixingRate","Accrual","EndPayment","StartSettlement"},{}};
    for(auto&r:x){double a=positive(r[1]),v=N*a*(r[0]-K),d=positive(1+a*r[0]);o.rows.push_back({r[0],a,v,v/d});}return o;
}

inline Report overnight_accrual_bridge(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);double l=0,s=0;Report o{{"Period","AccrualFactor","CompoundedReturn","SimpleAccrued","CompoundingGap"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double a=r[0]*positive(r[1]);positive(1+a);l+=std::log1p(a);s+=a;double v=std::expm1(l);o.rows.push_back({double(i+1),1+a,v,s,v-s});}return o;
}
} }
