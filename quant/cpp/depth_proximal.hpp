#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_proximal {
using namespace evidence;
inline Report group_prox(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);require(p[0]>=0,"Negative penalty");
    std::map<double,double> n;
    for(auto&r:x){require(r[0]==std::floor(r[0]),"Integer group required");n[r[0]]=std::hypot(n[r[0]],r[1]);}
    Report o{{"Group","Value","Prox","GroupNorm"},{}};
    for(auto&r:x){double v=n[r[0]],s=v==0?0:std::max(0.,1-p[0]/v);o.rows.push_back({r[0],r[1],s*r[1],v});}
    return o;
}

inline Report affine_projection(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);
    double s=0,t=0;
    for(auto&r:x){s+=r[0]*r[1];t+=r[1]*r[1];}
    double u=(s-p[0])/positive(t);
    Report o{{"Index","Projected","Multiplier"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0]-u*x[i][1],u});
    return o;
}

inline Report bounded_budget(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);
    double lsum=0,usum=0,lo=std::numeric_limits<double>::infinity(),hi=-lo;
    for(auto&r:x){
        positive(r[1]);require(r[2]<=r[3],"Invalid bounds");
        lsum+=r[2];usum+=r[3];
        lo=std::min(lo,r[0]-r[1]*r[3]);hi=std::max(hi,r[0]-r[1]*r[2]);
    }
    require(p[0]>=lsum-1e-12&&p[0]<=usum+1e-12,"Infeasible budget");
    auto w=[](const Vec&r,double e){return std::max(r[2],std::min(r[3],(r[0]-e)/r[1]));};
    for(int k=0;k<80;++k){double mid=lo/2+hi/2,s=0;for(auto&r:x)s+=w(r,mid);if(s>p[0])lo=mid;else hi=mid;}
    double eta=lo/2+hi/2;
    Report o{{"Asset","Weight","Multiplier","StationarityGap"},{}};
    for(std::size_t i=0;i<x.size();++i){
        auto&r=x[i];double z=w(r,eta),g=r[1]*z-r[0]+eta;
        double err=r[2]==r[3]?0:z==r[2]?std::max(0.,-g):z==r[3]?std::max(0.,g):std::abs(g);
        o.rows.push_back({double(i+1),z,eta,err});
    }
    return o;
}

inline Report huber_prox(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);require(p[0]>=0,"Negative proximal weight");double d=positive(p[1]),a=p[0];
    Report o{{"Input","Prox","Residual"},{}};
    for(auto&r:x){double z=r[0],v=std::abs(z)<=(1+a)*d?z/(1+a):z-std::copysign(a*d,z);o.rows.push_back({z,v,z-v});}
    return o;
}
} }
