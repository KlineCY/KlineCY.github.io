#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_sampling {
using namespace evidence;
inline Report reliability_variance(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);double s=0,m=0;
    for(auto&r:x){require(r[1]>=0,"Negative weight");s+=r[1];m+=r[1]*r[0];}m/=positive(s);
    double v=0,q=0;for(auto&r:x){double w=r[1]/s,d=r[0]-m;v+=w*d*d;q+=w*w;}
    return scalar({"WeightedMean","PopulationVariance","CorrectedVariance","EffectiveN"},{m,v,v/positive(1-q),1/q});
}

inline Report pairwise_missing(const Mat& x,const Vec& p){
    validate(x,2,4,4);parameters(p,0);double n=0,a=0,b=0;
    for(auto&r:x){require((r[2]==0||r[2]==1)&&(r[3]==0||r[3]==1),"Binary observation mask required");if(r[2]&&r[3]){++n;a+=r[0];b+=r[1];}}
    require(n>=2,"Two complete pairs required");a/=n;b/=n;double c=0;
    for(auto&r:x)if(r[2]&&r[3])c+=(r[0]-a)*(r[1]-b);
    return scalar({"Pairs","MeanX","MeanY","SampleCovariance"},{n,a,b,c/(n-1)});
}

inline Report group_jackknife(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);std::map<double,std::pair<double,std::size_t>> g;double s=0;
    for(auto&r:x){require(r[0]==std::floor(r[0]),"Integer group required");g[r[0]].first+=r[1];++g[r[0]].second;s+=r[1];}
    require(g.size()>=2,"Two groups required");auto k=g.begin()->second.second;double n=x.size(),m=s/n,v=0;
    Report o{{"Group","LeaveOneGroupMean","FullMean","JackknifeSE"},{}};
    for(auto&a:g){require(a.second.second==k,"Equal cluster sizes required");double z=(s-a.second.first)/(n-k);v+=(z-m)*(z-m);o.rows.push_back({a.first,z,m,0});}
    double se=std::sqrt(v*(g.size()-1)/g.size());for(auto&r:o.rows)r.back()=se;return o;
}

inline Report trim_winsor(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);auto n=x.size(),k=count(p[0],0,(n-1)/2);
    Vec v=column(x,0);double raw=mean(v);std::sort(v.begin(),v.end());double t=0;
    for(std::size_t i=k;i<n-k;++i)t+=v[i];t/=n-2*k;
    double lo=v[k],hi=v[n-k-1];for(double&z:v)z=std::max(lo,std::min(hi,z));
    double m=mean(v),s=0;for(double z:v)s+=(z-m)*(z-m);
    return scalar({"OriginalMean","TrimmedMean","WinsorMean","WinsorSampleVariance"},{raw,t,m,s/(n-1)});
}
} }
