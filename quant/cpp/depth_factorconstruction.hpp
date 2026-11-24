#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_factorconstruction {
using namespace evidence;
inline Report tie_aware_tail_weights(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);require(p[0]>0&&p[0]<=.5,"Invalid tail fraction");Vec s=column(x,0);auto ix=order(s);double m=p[0]*x.size();Vec L(x.size(),0),S(x.size(),0);
    auto fill=[&](std::vector<std::size_t> ids,Vec& w){double rem=m;for(std::size_t i=0;i<ids.size()&&rem>0;){std::size_t j=i+1;while(j<ids.size()&&s[ids[j]]==s[ids[i]])++j;double a=std::min(rem,double(j-i)),v=a/(j-i)/m;for(std::size_t k=i;k<j;++k)w[ids[k]]=v;rem-=a;i=j;}};
    fill(ix,S);std::reverse(ix.begin(),ix.end());fill(ix,L);Report o{{"Asset","Score","LongWeight","ShortWeight","NetWeight"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),s[i],L[i],S[i],L[i]-S[i]});return o;
}

inline Report size_value_six_cells(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);Vec w(6,0),v(6,0);for(auto&r:x){auto s=count(r[0],0,1),k=count(r[1],0,2),j=3*s+k;double a=positive(r[2]);require(r[3]>=-1,"Invalid simple return");w[j]+=a;v[j]+=a*r[3];}
    for(std::size_t j=0;j<6;++j)v[j]/=positive(w[j]);double smb=(v[0]+v[1]+v[2]-v[3]-v[4]-v[5])/3,hml=(v[2]+v[5]-v[0]-v[3])/2;v.push_back(smb);v.push_back(hml);return scalar({"SmallLow","SmallMid","SmallHigh","BigLow","BigMid","BigHigh","SMB","HML"},v);
}

inline Report beta_posterior_shrink(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);double m=p[0],v=positive(p[1]);Report o{{"EstimatedBeta","SamplingVariance","PosteriorMean","PosteriorVariance","DataWeight"},{}};
    for(auto&r:x){double s=positive(r[1]);s*=s;double a=v/(v+s);o.rows.push_back({r[0],s,a*r[0]+(1-a)*m,a*s,a});}return o;
}

inline Report constant_mix_diversification(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double w=p[0];require(w>=0&&w<=1,"Long-only target weight required");double a=1,b=1,v=1;Report o{{"Period","ConstantMixNAV","BuyHoldNAV","RebalanceExcess","PostReturnWeightA","OneWayTurnover"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double u=positive(1+r[0]),z=positive(1+r[1]),g=w*u+(1-w)*z;v*=g;a*=u;b*=z;double bh=w*a+(1-w)*b,d=w*u/g;o.rows.push_back({double(i+1),v,bh,v-bh,d,std::abs(d-w)});}return o;
}
} }
