#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_robustallocation {
using namespace evidence;
inline Report ellipsoid_worst_mean(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);require(p[0]>=0,"Negative uncertainty radius");double a=0,b=0;
    for(auto&r:x){require(r[2]>=0,"Negative uncertainty scale");a+=r[0]*r[1];b=std::hypot(b,r[0]*r[2]);}return scalar({"NominalReturn","UncertaintyNorm","WorstCaseReturn"},{a,b,a-p[0]*b});
}

inline Report stochastic_dominance(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double a=0,b=0,I=0;bool f=true,s=true;Report o{{"Outcome","CDFA","CDFB","IntegralDifference","AFirstOrder","ASecondOrder"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[1]>=0&&r[2]>=0,"Negative probability");if(i)I+=(a-b)*positive(r[0]-x[i-1][0]);a+=r[1];b+=r[2];f=f&&a<=b+1e-10;s=s&&I<=1e-10;o.rows.push_back({r[0],a,b,I,0,0});}
    require(std::abs(a-1)<1e-10&&std::abs(b-1)<1e-10,"Probability sums");for(auto&r:o.rows){r[4]=double(f);r[5]=double(s);}return o;
}

inline Report single_asset_no_trade(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,3);double c=p[0],l=p[1],u=p[2];require(c>=0&&l<=u,"Invalid cost or bounds");Report o{{"Current","Target","NewWeight","Trade","Cost"},{}};
    for(auto&r:x){double a=positive(r[2]),w=r[0],z=r[1];require(w>=l&&w<=u,"Current weight infeasible");double d=z-w,v=w+std::copysign(std::max(0.,std::abs(d)-c/a),d);v=std::max(l,std::min(u,v));o.rows.push_back({w,z,v,v-w,c*std::abs(v-w)});}return o;
}

inline Report mean_estimation_risk(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,1);require(x.size()==d,"Square covariance required");auto n=count(p[0],1,1000000000);Mat s;Vec w;
    for(auto&r:x){s.emplace_back(r.begin(),r.end()-1);w.push_back(r.back());}solve_spd(s,Vec(d,0));double v=dot(w,mul(s,w));require(v>=0,"Negative portfolio variance");return scalar({"PortfolioVariance","MeanEstimateVariance","MeanEstimateSE"},{v,v/n,std::sqrt(v/n)});
}
} }
