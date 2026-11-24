#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_uncertainty {
using namespace evidence;
inline Report mixture_predictive_variance(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double s=0,m=0,a=0;
    for(auto&r:x){require(r[0]>=0&&r[2]>=0,"Invalid weight or variance");s+=r[0];m+=r[0]*r[1];a+=r[0]*r[2];}require(std::abs(s-1)<1e-10,"Weight sum");double b=0;for(auto&r:x)b+=r[0]*(r[1]-m)*(r[1]-m);
    return scalar({"PredictiveMean","WithinVariance","BetweenVariance","TotalVariance"},{m,a,b,a+b});
}

inline Report ensemble_entropy_mi(const Mat& x,const Vec& p){
    auto K=validate(x,1,3,64)-1;parameters(p,0);Vec q(K,0);double s=0,H=0;
    for(auto&r:x){require(r[0]>=0,"Negative weight");s+=r[0];double a=0,h=0;for(std::size_t k=0;k<K;++k){double v=r[k+1];require(v>=0&&v<=1,"Invalid probability");a+=v;q[k]+=r[0]*v;if(v>0)h-=v*std::log(v);}require(std::abs(a-1)<1e-10,"Probability sum");H+=r[0]*h;}
    require(std::abs(s-1)<1e-10,"Weight sum");double v=0;for(double a:q)if(a>0)v-=a*std::log(a);
    return scalar({"PredictiveEntropy","MeanConditionalEntropy","MutualInformation"},{v,H,std::max(0.,v-H)});
}

inline Report mondrian_calibration(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);require(p[0]>0&&p[0]<1,"Invalid alpha");std::map<double,Vec> g;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]>=0,"Invalid group or score");g[r[0]].push_back(r[1]);}
    Report o{{"Group","CalibrationCount","Rank","Threshold"},{}};
    for(auto&z:g){auto&v=z.second;std::sort(v.begin(),v.end());auto n=v.size(),k=std::size_t(std::ceil((n+1)*(1-p[0])));require(k>=1&&k<=n,"Infinite threshold: insufficient calibration observations");o.rows.push_back({z.first,double(n),double(k),v[k-1]});}return o;
}

inline Report wilson_coverage(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double z=positive(p[0]),a=z*z;Report o{{"Successes","Trials","Observed","Lower","Upper"},{}};
    for(auto&r:x){double n=count(r[1],1,1000000000),k=count(r[0],0,std::size_t(n)),q=k/n,d=1+a/n,c=(q+a/(2*n))/d,h=z*std::sqrt(q*(1-q)/n+a/(4*n*n))/d;o.rows.push_back({k,n,q,std::max(0.,c-h),std::min(1.,c+h)});}return o;
}
} }
