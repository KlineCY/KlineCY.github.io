#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec online_logistic(const Mat& x,const Vec& p){
    inputs(x,p,3,1);double lr=positive(p[0]),a=0,b=0,loss=0;
    for(const auto& r:x){require(r[2]==0||r[2]==1,"Binary label required");double z=a*r[0]+b*r[1],q=logistic(z);loss+=std::max(z,0.0)-r[2]*z+std::log1p(std::exp(-std::abs(z)));a-=lr*(q-r[2])*r[0];b-=lr*(q-r[2])*r[1];}
    return checked({a,b,loss/x.size()});
}

inline Vec adagrad_ridge(const Mat& x,const Vec& p){
    inputs(x,p,3,3);double lr=positive(p[0]),l=p[1],eps=positive(p[2]);require(l>=0,"Negative regularization");
    Vec w(2,0),g2(2,0);double mse=0;
    for(const auto& r:x){double e=w[0]*r[0]+w[1]*r[1]-r[2];mse+=e*e;for(std::size_t j=0;j<2;++j){double g=e*r[j]+l*w[j];g2[j]+=g*g;w[j]-=lr*g/(std::sqrt(g2[j])+eps);}}
    return checked({w[0],w[1],mse/x.size()});
}

inline Vec brier_decomposition(const Mat& x,const Vec& p){
    inputs(x,p,2,0);std::map<double,Vec> groups;double bs=0,base=0,n=double(x.size());
    for(const auto& r:x){require(r[0]>=0&&r[0]<=1&&(r[1]==0||r[1]==1),"Invalid probability or label");double e=r[0]-r[1];bs+=e*e;base+=r[1];auto& g=groups[r[0]];if(g.empty())g={0,0};g[0]+=r[1];++g[1];}
    base/=n;double rel=0,res=0;
    for(const auto& g:groups){double o=g.second[0]/g.second[1],w=g.second[1]/n;rel+=w*(g.first-o)*(g.first-o);res+=w*(o-base)*(o-base);}
    return checked({bs/n,rel,res,base*(1-base)});
}

inline Vec isotonic_pav(const Mat& x,const Vec& p){
    inputs(x,p,2,0);struct Block{std::size_t count;double s;};std::vector<Block> blocks;
    for(std::size_t i=0;i<x.size();){require(i==0||x[i][0]>=x[i-1][0],"Scores must be sorted");std::size_t j=i;double s=0;
        while(j<x.size()&&x[j][0]==x[i][0]){require(x[j][1]==0||x[j][1]==1,"Binary label required");s+=x[j][1];++j;}
        blocks.push_back({j-i,s});i=j;
        while(blocks.size()>=2){auto b=blocks.back(),a=blocks[blocks.size()-2];if(a.s/a.count<=b.s/b.count)break;blocks.pop_back();blocks.back()={a.count+b.count,a.s+b.s};}
    }
    Vec out;for(auto b:blocks)out.insert(out.end(),b.count,b.s/b.count);return checked(out);
}

inline Vec stacking_two(const Mat& x,const Vec& p){
    inputs(x,p,3,0);double a=0,b=0;
    for(const auto& r:x){double d=r[0]-r[1];a+=d*(r[2]-r[1]);b+=d*d;}
    double w=b>0?std::clamp(a/b,0.0,1.0):0.5,mse=0;
    for(const auto& r:x){double e=w*r[0]+(1-w)*r[1]-r[2];mse+=e*e;}
    return checked({w,1-w,mse/x.size()});
}
} }
