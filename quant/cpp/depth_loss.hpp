#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_loss {
using namespace evidence;
inline Report binary_focal(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double g=p[0];require(g>=0,"Negative focal gamma");Report o{{"Logit","Label","Loss","Derivative"},{}};
    for(auto&r:x){
        require(r[1]==0||r[1]==1,"Binary label required");
        double sign=2*r[1]-1,t=sign*r[0],lp=t>=0?-std::log1p(std::exp(-t)):t-std::log1p(std::exp(t));
        double q=-std::expm1(lp),pt=std::exp(lp),f=g==0?1:std::pow(q,g);
        o.rows.push_back({r[0],r[1],-f*lp,sign*f*(g*pt*lp-q)});
    }return o;
}

inline Report huberized_pinball(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);require(p[0]>0&&p[0]<1,"Invalid quantile");double d=positive(p[1]);Report o{{"Residual","Loss","ResidualGradient"},{}};
    for(auto&r:x){double u=r[0],a=std::abs(u),w=u<0?1-p[0]:p[0],h=a<=d?a*a/(2*d):a-d/2,g=std::max(-1.,std::min(1.,u/d));o.rows.push_back({u,w*h,w*g});}return o;
}

inline Report logit_intercept_score(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double L=0,g=0,h=0;
    for(auto&r:x){require(r[1]==0||r[1]==1,"Binary label required");double z=r[0]+p[0],q=z>=0?1/(1+std::exp(-z)):std::exp(z)/(1+std::exp(z));L+=std::max(z,0.)-r[1]*z+std::log1p(std::exp(-std::abs(z)));g+=q-r[1];h+=q*(1-q);}
    return scalar({"NLL","Score","Information","NewtonStep"},{L,g,h,p[0]-g/positive(h)});
}

inline Report temperature_curvature(const Mat& x,const Vec& p){
    auto K=validate(x,1,3,64)-1;parameters(p,1);double b=p[0];require(b>=0,"Negative inverse temperature");double L=0,g=0,h=0;
    for(auto&r:x){auto y=count(r[0],0,K-1);double m=*std::max_element(r.begin()+1,r.end()),s=0,u=0,v=0;
        for(std::size_t j=0;j<K;++j){double z=r[j+1]-m,w=std::exp(b*z);s+=w;u+=w*z;v+=w*z*z;}
        L+=std::log(s)-b*(r[y+1]-m);u/=s;g+=u-(r[y+1]-m);h+=std::max(0.,v/s-u*u);
    }return scalar({"InverseTemperature","NLL","Gradient","Curvature"},{b,L,g,h});
}
} }
