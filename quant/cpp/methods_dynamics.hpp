#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_dynamics {
using namespace evidence;
inline Report scalar_riccati(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,4);
    double phi=p[0],q=positive(p[1]),r=positive(p[2]),v=p[3];
    require(v>=0,"Negative initial variance");
    Report o{{"Time","PredictionVariance","Gain","PosteriorVariance"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){double a=phi*phi*v+q,k=a/(a+r);
    v=(1-k)*(1-k)*a+k*k*r;
    o.rows.push_back({double(i+1),a,k,v});
    }return o;
    
}

inline Report ou_transition(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,4);
    double k=positive(p[0]),theta=p[1],s=positive(p[2]),z=p[3];
    Report o{{"Horizon","Mean","Variance"},{}};
    for(auto&r:x){require(r[0]>=0,"Negative horizon");
    o.rows.push_back({r[0],theta+(z-theta)*std::exp(-k*r[0]),-s*s*std::expm1(-2*k*r[0])/(2*k)});
    }return o;
    
}

inline Report markov_occupation(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,3);
    double a=p[0],b=p[1],z=p[2];
    require(a>=0&&a<=1&&b>=0&&b<=1&&a+b>0&&z>=0&&z<=1,"Invalid transition or initial probability");
    double s=a/(a+b);
    Report o{{"Horizon","StateOne","StationaryOne"},{}};
    for(auto&r:x){auto h=count(r[0],0,1000000);
    o.rows.push_back({double(h),s+(z-s)*std::pow(1-a-b,double(h)),s});
    }return o;
    
}

inline Report innovation_likelihood(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    double l=0,s=0;
    for(auto&r:x){double v=positive(r[1]),q=r[0]*r[0]/v;
    s+=q;
    l-=.5*(std::log(2*std::acos(-1.))+std::log(v)+q);
    }return scalar({"LogLikelihood","StandardizedSS"},{l,s});
    
}
} }
