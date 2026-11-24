#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_learning {
using namespace evidence;
inline Report kernel_alignment(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    double a=mean(column(x,0)),b=mean(column(x,1)),s=0,t=0,c=0;
    for(auto&r:x){double u=r[0]-a,v=r[1]-b;
    s+=u*u;
    t+=v*v;
    c+=u*v;
    }return scalar({"Alignment","KernelNorm","TargetNorm"},{c*c/(positive(s)*positive(t)),s,t});
    
}

inline Report gram_centering(const Mat& x,const Vec& p){
    auto n=validate(x,2,2,64);
    parameters(p,0);
    require(x.size()==n,"Square Gram matrix required");
    Vec m(n,0);
    double a=0;
    for(std::size_t i=0;
    i<n;
    ++i)for(std::size_t j=0;
    j<n;
    ++j){require(std::abs(x[i][j]-x[j][i])<=1e-10*std::max({1.,std::abs(x[i][j]),std::abs(x[j][i])}),"Symmetric Gram matrix required");
    m[i]+=x[i][j]/n;
    a+=x[i][j]/double(n*n);
    }Report o{{"Row"},{}};
    for(std::size_t j=0;
    j<n;
    ++j)o.header.push_back("Kc"+std::to_string(j+1));
    for(std::size_t i=0;
    i<n;
    ++i){Vec r{double(i+1)};
    for(std::size_t j=0;
    j<n;
    ++j)r.push_back(x[i][j]-m[i]-m[j]+a);
    o.rows.push_back(r);
    }return o;
    
}

inline Report soft_label(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    double a=0,b=0,ce=0,h=0;
    for(auto&r:x){require(r[0]>0&&r[0]<=1&&r[1]>=0&&r[1]<=1,"Invalid probability");
    a+=r[0];
    b+=r[1];
    if(r[1]>0){ce-=r[1]*std::log(r[0]);
    h-=r[1]*std::log(r[1]);
    }}require(std::abs(a-1)<1e-10&&std::abs(b-1)<1e-10,"Probabilities must sum to one");
    return scalar({"CrossEntropy","TargetEntropy","KL"},{ce,h,ce-h});
    
}

inline Report calibration_slope(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    double a=mean(column(x,0)),b=mean(column(x,1)),s=0,c=0;
    for(auto&r:x){require(r[0]>=0&&r[0]<=1&&(r[1]==0||r[1]==1),"Invalid forecast or event");
    s+=(r[0]-a)*(r[0]-a);
    c+=(r[0]-a)*(r[1]-b);
    }double beta=c/positive(s);
    return scalar({"Intercept","Slope","MeanForecast","MeanOutcome"},{b-beta*a,beta,a,b});
    
}
} }
