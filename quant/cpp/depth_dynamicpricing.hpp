#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_dynamicpricing {
using namespace evidence;
inline Report node_martingale(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,0);std::map<double,Vec> g;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]>=0&&r[2]>=0&&r[3]>=0,"Invalid node data");positive(r[4]);if(!g.count(r[0]))g[r[0]]={r[1],r[4],0,0};auto&v=g[r[0]];require(v[0]==r[1]&&v[1]==r[4],"Inconsistent parent or discount");v[2]+=r[3];v[3]+=r[3]*r[2];}
    Report o{{"Node","Parent","DiscountedChildMean","MartingaleGap"},{}};
    for(auto&z:g){auto&v=z.second;require(std::abs(v[2]-1)<1e-10,"Conditional probability sum");double a=v[1]*v[3];o.rows.push_back({z.first,v[0],a,a-v[0]});}return o;
}

inline Report numeraire_measure(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double a=0,b=0;Report o{{"Scenario","NewProbability","BaseWeightedPayoff","NewWeightedPayoff"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]>=0,"Negative base probability");double z=positive(r[1]),q=r[0]*z;a+=r[0];b+=q;o.rows.push_back({double(i+1),q,r[0]*z*r[2],q*r[2]});}require(std::abs(a-1)<1e-10&&std::abs(b-1)<1e-10,"Probability or density normalization");return o;
}

inline Report stopping_policy_value(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,63);parameters(p,0);require(d%2==1,"Payoff and exercise dimensions differ");auto T=(d-1)/2;double s=0;Report o{{"Path","ExerciseTime","DiscountedPayoff","RunningMean"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];std::size_t k=T;for(std::size_t j=0;j<T;++j){double a=r[T+1+j];require(a==0||a==1,"Binary policy flag required");if(a&&k==T)k=j;}double v=r[k];s+=v;o.rows.push_back({double(i+1),double(k),v,s/(i+1)});}return o;
}

inline Report dual_stopping_bound(const Mat& x,const Vec& p){
    auto d=validate(x,1,4,64);parameters(p,0);require(d%2==0,"Equal payoff and martingale columns required");auto n=d/2;double s=0;Report o{{"Path","PathwiseUpper","RunningMean"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[n]==0,"Martingale must start at zero");double v=r[0];for(std::size_t j=1;j<n;++j)v=std::max(v,r[j]-r[n+j]);s+=v;o.rows.push_back({double(i+1),v,s/(i+1)});}return o;
}
} }
