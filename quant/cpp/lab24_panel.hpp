#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec within_panel(const Mat& x,const Vec& p){
    inputs(x,p,3,0,3);std::map<std::size_t,Vec> groups;
    for(const auto& r:x){auto id=integer(r[0],0,1000000000);auto& g=groups[id];if(g.empty())g={0,0,0};g[0]+=r[1];g[1]+=r[2];++g[2];}
    double xx=0,xy=0,yy=0;
    for(const auto& r:x){const auto& g=groups.at(std::size_t(r[0]));double a=r[1]-g[0]/g[2],b=r[2]-g[1]/g[2];xx+=a*a;xy+=a*b;yy+=b*b;}
    double beta=xy/positive(xx);return checked({beta,xy*xy/(xx*positive(yy))});
}

inline Vec cluster_ols(const Mat& x,const Vec& p){
    inputs(x,p,3,0,3);double mx=mean(col(x,1)),my=mean(col(x,2)),xx=0,xy=0;
    for(const auto& r:x){integer(r[0],0,1000000000);xx+=(r[1]-mx)*(r[1]-mx);xy+=(r[1]-mx)*(r[2]-my);}
    double b=xy/positive(xx),a=my-b*mx;std::map<std::size_t,double> score;
    for(const auto& r:x)score[std::size_t(r[0])]+=(r[1]-mx)*(r[2]-a-b*r[1]);
    require(score.size()>=2,"At least two clusters required");
    double v=0;for(auto g:score)v+=g.second*g.second;
    double G=double(score.size()),n=double(x.size());v*=G/(G-1)*(n-1)/(n-2)/(xx*xx);
    return checked({a,b,std::sqrt(v)});
}

inline Vec paired_t(const Mat& x,const Vec& p){
    inputs(x,p,2,0,2);Vec d;for(const auto& r:x)d.push_back(r[1]-r[0]);
    double m=mean(d),se=positive(std::sqrt(variance(d)/d.size()));return checked({m,se,m/se});
}

inline Vec randomization_exact(const Mat& x,const Vec& p){
    inputs(x,p,1,0);require(x.size()<=18,"Exact enumeration limited to 18 pairs");Vec d=col(x,0);
    double obs=std::abs(sum(d));std::size_t N=std::size_t(1)<<d.size(),hits=0;
    for(std::size_t k=0;k<N;++k){double z=0;for(std::size_t i=0;i<d.size();++i)z+=((k>>i)&1)?d[i]:-d[i];if(std::abs(z)+1e-12*std::max(1.0,obs)>=obs)++hits;}
    return checked({mean(d),double(hits)/N});
}

inline Vec iv_first_stage(const Mat& x,const Vec& p){
    inputs(x,p,3,0,3);Vec z=col(x,0),d=col(x,1),y=col(x,2);
    double mz=mean(z),md=mean(d),zz=0,zd=0,zy=0,my=mean(y);
    for(std::size_t i=0;i<x.size();++i){double a=z[i]-mz;zz+=a*a;zd+=a*(d[i]-md);zy+=a*(y[i]-my);}
    require(std::abs(zd)>0,"Irrelevant instrument");double b=zd/positive(zz),sse=0;
    for(std::size_t i=0;i<x.size();++i){double e=d[i]-md-b*(z[i]-mz);sse+=e*e;}
    double se2=positive(sse/(x.size()-2)/zz);
    return checked({b,b*b/se2,zy/zd});
}
} }
