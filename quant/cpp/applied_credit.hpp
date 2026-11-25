#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_credit {
using namespace evidence;
inline Report competing_incidence(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    std::map<double,Vec> g;
    for(auto&r:x){positive(r[0]);
    auto e=count(r[1],0,2);
    if(!g.count(r[0]))g[r[0]]=Vec(3,0);
    g[r[0]][e]++;
    }double n=x.size(),s=1,a=0,b=0;
    Report o{{"Time","AtRisk","Event1","Event2","Censored","Survival","CIF1","CIF2"},{}};
    for(auto&z:g){auto&r=z.second;
    a+=s*r[1]/n;
    b+=s*r[2]/n;
    s*=1-(r[1]+r[2])/n;
    o.rows.push_back({z.first,n,r[1],r[2],r[0],s,a,b});
    n-=sum(r);
    }return o;
    
}

inline Report exposure_profile(const Mat& x,const Vec& p){
    auto n=validate(x,1,2,64);
    parameters(p,1);
    require(p[0]>0&&p[0]<=1,"Invalid quantile");
    auto k=std::size_t(std::ceil(p[0]*n))-1;
    Report o{{"Horizon","ExpectedPositiveExposure","PFE"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){Vec v=x[i];
    for(double&z:v)z=std::max(z,0.);
    double a=mean(v);
    std::sort(v.begin(),v.end());
    o.rows.push_back({double(i+1),a,v[k]});
    }return o;
    
}

inline Report wrong_way(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    require(p[0]>=0&&p[0]<=1,"Invalid LGD");
    double a=0,b=0,c=0,n=x.size();
    for(auto&r:x){require(r[0]>=0&&r[1]>=0&&r[1]<=1,"Invalid exposure or PD");
    a+=r[0]/n;
    b+=r[1]/n;
    c+=r[0]*r[1]/n;
    }return scalar({"ExpectedExposure","MeanPD","JointLoss","IndependentLoss","DependenceAddOn"},{a,b,p[0]*c,p[0]*a*b,p[0]*(c-a*b)});
    
}

inline Report netting(const Mat& x,const Vec& p){
    auto n=validate(x,1,3,64);
    parameters(p,0);
    Report o{{"Scenario","GrossPositive","NetPositive","Collateralized","NettingBenefit","CollateralBenefit"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    require(r.back()>=0,"Negative collateral");
    double a=0,b=0;
    for(std::size_t j=0;
    j+1<n;
    ++j){a+=std::max(r[j],0.);
    b+=r[j];
    }double net=std::max(b,0.),c=std::max(b-r.back(),0.);
    o.rows.push_back({double(i+1),a,net,c,a-net,net-c});
    }return o;
    
}
} }
