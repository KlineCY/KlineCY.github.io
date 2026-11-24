#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_features {
using namespace evidence;
inline Report weighted_group_demean(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);std::map<double,std::pair<double,double>> g;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[2]>=0,"Invalid group or weight");g[r[0]].first+=r[1]*r[2];g[r[0]].second+=r[2];}
    Report o{{"Group","Value","GroupMean","Residual","Weight"},{}};
    for(auto&r:x){auto&v=g[r[0]];double m=v.first/positive(v.second);o.rows.push_back({r[0],r[1],m,r[1]-m,r[2]});}return o;
}

inline Report train_missing_indicator(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double s=0,n=0;
    for(auto&r:x){require((r[0]==0||r[0]==1)&&(r[1]==0||r[1]==1),"Binary flags required");if(r[0]&&r[1]){s+=r[2];++n;}}
    double m=s/positive(n);Report o{{"Row","TrainFlag","Imputed","MissingIndicator","TrainingMean"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];o.rows.push_back({double(i+1),r[0],r[1]?r[2]:m,1-r[1],m});}return o;
}

inline Report ew_prior_zscore(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,3);double a=p[0],m=p[1],v=positive(p[2]);require(a>0&&a<1,"Invalid decay weight");Report o{{"Time","PriorMean","PriorVariance","ZScore","UpdatedMean","UpdatedVariance"},{}};
    for(std::size_t i=0;i<x.size();++i){double e=x[i][0]-m,z=e/std::sqrt(positive(v)),u=m+a*e,w=(1-a)*v+a*(1-a)*e*e;o.rows.push_back({double(i+1),m,v,z,u,w});m=u;v=w;}return o;
}

inline Report regime_covariance(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);std::map<double,Vec> g;double n=x.size(),a=mean(column(x,1)),b=mean(column(x,2));
    for(auto&r:x){require(r[0]==std::floor(r[0]),"Integer regime required");if(!g.count(r[0]))g[r[0]]=Vec(3,0);auto&v=g[r[0]];++v[0];v[1]+=r[1];v[2]+=r[2];}
    for(auto&z:g){z.second[1]/=z.second[0];z.second[2]/=z.second[0];}
    double w=0,c=0;for(auto&r:x){auto&v=g[r[0]];w+=(r[1]-v[1])*(r[2]-v[2])/n;}
    for(auto&z:g){auto&v=z.second;c+=v[0]*(v[1]-a)*(v[2]-b)/n;}
    return scalar({"MeanX","MeanY","WithinCovariance","BetweenCovariance","TotalCovariance"},{a,b,w,c,w+c});
}
} }
