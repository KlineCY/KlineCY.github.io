#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_validation {
using namespace evidence;
inline Report purge_interval(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,3);require(p[0]<p[1]&&p[2]>=0,"Invalid test interval");Report o{{"Row","Start","End","TestOverlap","Embargo","TrainEligible"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]<r[1],"Invalid label interval");bool a=r[0]<p[1]&&r[1]>p[0],b=r[0]>=p[1]&&r[0]<p[1]+p[2];o.rows.push_back({double(i+1),r[0],r[1],double(a),double(b),double(!a&&!b)});}return o;
}

inline Report train_only_standardization(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);double n=0,m=0;
    for(auto&r:x){require(r[0]==0||r[0]==1,"Binary train flag required");if(r[0]){++n;m+=r[1];}}
    require(n>=2,"Two training values required");m/=n;double v=0;for(auto&r:x)if(r[0])v+=(r[1]-m)*(r[1]-m)/n;double s=std::sqrt(positive(v));
    Report o{{"Row","TrainFlag","Value","TrainingMean","TrainingScale","Standardized"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0],x[i][1],m,s,(x[i][1]-m)/s});return o;
}

inline Report forecast_oos_r2(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double a=0,b=0;
    for(auto&r:x){a+=(r[0]-r[1])*(r[0]-r[1]);b+=(r[0]-r[2])*(r[0]-r[2]);}double q=a/positive(b);return scalar({"ModelSSE","BenchmarkSSE","OOSR2","MSERatio"},{a,b,1-q,q});
}

inline Report block_loss_decomposition(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);std::map<double,Vec> a;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]>=0&&r[2]>=0,"Invalid block or loss");if(!a.count(r[0]))a[r[0]]=Vec(3,0);auto&v=a[r[0]];++v[0];v[1]+=r[1];v[2]+=r[2];}
    Report o{{"Block","Count","MeanModelLoss","MeanBenchmarkLoss","MeanImprovement","Contribution"},{}};
    for(auto&z:a){auto&v=z.second;double d=v[2]-v[1];o.rows.push_back({z.first,v[0],v[1]/v[0],v[2]/v[0],d/v[0],d/x.size()});}return o;
}
} }
