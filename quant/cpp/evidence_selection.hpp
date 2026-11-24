#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report fdr_adjustment(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);require(p[0]>0&&p[0]<1,"Invalid FDR level");
    Vec v=column(x,0),bh(v.size()),by(v.size());auto ix=order(v);double H=0,m=double(v.size());
    for(std::size_t j=0;j<v.size();++j){require(v[j]>=0&&v[j]<=1,"Invalid p-value");H+=1.0/(j+1);}
    double q=1;
    for(std::size_t j=ix.size();j-->0;){q=std::min(q,m*v[ix[j]]/(j+1));bh[ix[j]]=q;by[ix[j]]=std::min(1.0,H*q);}
    Report o{{"Hypothesis","PValue","BHAdjusted","BYAdjusted","BHReject","BYReject"},{}};
    for(std::size_t j=0;j<v.size();++j)o.rows.push_back({double(j+1),v[j],bh[j],by[j],double(bh[j]<=p[0]),double(by[j]<=p[0])});
    return o;
}

inline Report holm_adjustment(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);require(p[0]>0&&p[0]<1,"Invalid FWER level");
    Vec v=column(x,0),adj(v.size());auto ix=order(v);double a=0;
    for(std::size_t j=0;j<ix.size();++j){double u=v[ix[j]];require(u>=0&&u<=1,"Invalid p-value");a=std::max(a,std::min(1.0,u*(ix.size()-j)));adj[ix[j]]=a;}
    Report o{{"Hypothesis","PValue","HolmAdjusted","Reject"},{}};
    for(std::size_t j=0;j<v.size();++j)o.rows.push_back({double(j+1),v[j],adj[j],double(adj[j]<=p[0])});
    return o;
}

inline Report cscv_mean_pbo(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,32);parameters(p,1);auto B=count(p[0],2,16);
    require(B%2==0&&x.size()%B==0,"Even equal-length blocks required");
    require(double(std::uint64_t(1)<<B)*B*d<=5e7,"CSCV work limit exceeded");
    Mat block(B,Vec(d,0));Vec total(d,0);auto L=x.size()/B;
    for(std::size_t t=0;t<x.size();++t)for(std::size_t j=0;j<d;++j){block[t/L][j]+=x[t][j];total[j]+=x[t][j];}
    Report o{{"Split","TrainMask","SelectedStrategy","TrainMean","TestMean","TestRank","RankFraction","Logit","BelowMedian"},{}};
    for(std::uint64_t mask=1;mask<(std::uint64_t(1)<<B);++mask){
        if(bits(mask)!=B/2)continue;
        Vec a(d,0),b(d,0);
        for(std::size_t k=0;k<B;++k)if(mask&(std::uint64_t(1)<<k))for(std::size_t j=0;j<d;++j)a[j]+=block[k][j];
        for(std::size_t j=0;j<d;++j)b[j]=total[j]-a[j];
        auto j=std::size_t(std::max_element(a.begin(),a.end())-a.begin());double rank=1;
        for(std::size_t k=0;k<d;++k)if(k!=j){if(b[k]<b[j])rank+=1;else if(b[k]==b[j])rank+=.5;}
        double w=rank/(d+1),z=std::log(w/(1-w));
        o.rows.push_back({double(o.rows.size()+1),double(mask),double(j+1),a[j]/(x.size()/2),b[j]/(x.size()/2),rank,w,z,double(z<0)});
    }
    return o;
}

inline Report max_signflip(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,32);parameters(p,0);auto T=x.size();
    require(T<=20&&double(std::uint64_t(1)<<T)*T*d<=5e7,"Exact sign-flip work limit exceeded");
    Vec obs=means(x),single(d,0),joint(d,0);
    for(double& v:obs)v=std::abs(v);
    auto M=std::uint64_t(1)<<T;
    double scale=0;for(const auto& row:x)for(double v:row)scale=std::max(scale,std::abs(v));
    for(std::uint64_t mask=0;mask<M;++mask){
        Vec z(d,0);for(std::size_t t=0;t<T;++t)for(std::size_t j=0;j<d;++j)z[j]+=(mask&(std::uint64_t(1)<<t)?1:-1)*x[t][j]/T;
        double mx=0;for(double& v:z){v=std::abs(v);mx=std::max(mx,v);}
        for(std::size_t j=0;j<d;++j){if(z[j]+1e-12*scale>=obs[j])++single[j];if(mx+1e-12*scale>=obs[j])++joint[j];}
    }
    Report o{{"Strategy","ObservedAbsMean","MarginalP","MaxAdjustedP","Permutations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),obs[j],single[j]/M,joint[j]/M,double(M)});
    return o;
}
} }
