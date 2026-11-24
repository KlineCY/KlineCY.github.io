#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_linear {
using namespace evidence;
inline Report hc3(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,0);
    double n=x.size(),a=mean(column(x,0)),b=mean(column(x,1)),s=0,c=0;
    for(auto&r:x){s+=(r[0]-a)*(r[0]-a);
    c+=(r[0]-a)*(r[1]-b);
    }positive(s);
    double beta=c/s,alpha=b-beta*a,v=0;
    for(auto&r:x){double d=r[0]-a,e=r[1]-alpha-beta*r[0],h=1/n+d*d/s;
    positive(1-h);
    v+=d*d*e*e/((1-h)*(1-h));
    }return scalar({"Intercept","Slope","HC3SE"},{alpha,beta,std::sqrt(v)/s});
    
}

inline Report press(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,0);
    double n=x.size(),a=mean(column(x,0)),b=mean(column(x,1)),s=0,c=0;
    for(auto&r:x){s+=(r[0]-a)*(r[0]-a);
    c+=(r[0]-a)*(r[1]-b);
    }positive(s);
    double beta=c/s,alpha=b-beta*a;
    Report o{{"Index","Fitted","Residual","Leverage","LOOResidual"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){double d=x[i][0]-a,f=alpha+beta*x[i][0],e=x[i][1]-f,h=1/n+d*d/s;
    positive(1-h);
    o.rows.push_back({double(i+1),f,e,h,e/(1-h)});
    }return o;
    
}

inline Report partial_r2(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,2);
    auto q=count(p[0],1,x.size()-1),k=count(p[1],q+1,x.size()-1);
    double a=0,b=0;
    for(auto&r:x){a+=r[0]*r[0];
    b+=r[1]*r[1];
    }positive(a);
    positive(b);
    require(a>=b,"Nested SSE required");
    return scalar({"RestrictedSSE","FullSSE","PartialR2","F"},{a,b,1-b/a,(a-b)/q/(b/(x.size()-k))});
    
}

inline Report vif(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,0);
    double a=mean(column(x,0)),b=mean(column(x,1)),v=0,w=0,c=0;
    for(auto&r:x){double d=r[0]-a,e=r[1]-b;
    v+=d*d;
    w+=e*e;
    c+=d*e;
    }double rho=c/std::sqrt(positive(v)*positive(w)),r2=rho*rho;
    require(1-r2>1e-14,"Collinear regressors");
    return scalar({"Correlation","R2","VIF"},{rho,r2,1/(1-r2)});
    
}
} }
