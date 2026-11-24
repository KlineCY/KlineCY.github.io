#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_identification {
using namespace evidence;
inline Report unbalanced_two_way_projection(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,2);double tol=positive(p[0]);auto K=count(p[1],1,100000);require(double(K)*x.size()<=5e7,"Projection work limit");std::map<double,std::vector<std::size_t>> e,t;std::set<std::pair<double,double>> keys;Vec r;
    for(std::size_t i=0;i<x.size();++i){auto&a=x[i];require(a[0]==std::floor(a[0])&&a[1]==std::floor(a[1])&&keys.emplace(a[0],a[1]).second,"Invalid or duplicate panel key");e[a[0]].push_back(i);t[a[1]].push_back(i);r.push_back(a[2]);}
    auto project=[&](const auto& g){for(auto&z:g){double m=0;for(auto i:z.second)m+=r[i]/z.second.size();for(auto i:z.second)r[i]-=m;}};
    auto gap=[&](const auto& g){double a=0;for(auto&z:g){double m=0;for(auto i:z.second)m+=r[i]/z.second.size();a=std::max(a,std::abs(m));}return a;};
    double err=std::numeric_limits<double>::infinity();for(std::size_t k=0;k<K;++k){project(e);project(t);err=std::max(gap(e),gap(t));if(err<=tol)break;}require(err<=tol,"Projection did not converge");
    Report o{{"Row","Entity","Time","Residual","MaxGroupMean"},{}};for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0],x[i][1],r[i],err});return o;
}

inline Report manski_missing_bounds(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);require(p[0]<=p[1],"Invalid outcome support");double n=0,s=0;for(auto&r:x){require(r[0]==0||r[0]==1,"Binary observation mask required");if(r[0]){require(r[1]>=p[0]&&r[1]<=p[1],"Observed outcome outside support");++n;s+=r[1];}}
    double m=x.size()-n;return scalar({"ObservedCount","MissingCount","ObservedMean","LowerMean","UpperMean"},{n,m,s/positive(n),(s+m*p[0])/x.size(),(s+m*p[1])/x.size()});
}

inline Report lee_trimming_bounds(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,0);double n1=0,n0=0;Vec a,b;for(auto&r:x){require((r[0]==0||r[0]==1)&&(r[1]==0||r[1]==1),"Binary flags required");if(r[0]){++n1;if(r[1])a.push_back(r[2]);}else{++n0;if(r[1])b.push_back(r[2]);}}
    require(n1>0&&n0>0&&!a.empty()&&!b.empty(),"Both selected groups required");double s1=a.size()/n1,s0=b.size()/n0;require(s1>=s0,"Treatment selection must be weakly greater");double q=s0/s1,m=q*a.size(),c=mean(b);std::sort(a.begin(),a.end());double lo=0,hi=0,left=m;
    for(std::size_t i=0;i<a.size()&&left>0;++i){double w=std::min(1.,left);lo+=w*a[i];hi+=w*a[a.size()-1-i];left-=w;}
    return scalar({"TreatmentSelection","ControlSelection","RetainFraction","ControlMean","LowerATE","UpperATE"},{s1,s0,q,c,lo/m-c,hi/m-c});
}

inline Report synthetic_control_gap(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,64)-2;parameters(p,d);double s=0;for(double w:p){require(w>=0,"Negative donor weight");s+=w;}require(std::abs(s-1)<1e-10,"Donor weights must sum to one");double a=0,b=0,c=0,n=0,m=0;bool post=false;
    for(auto&r:x){require(r[0]==0||r[0]==1,"Binary post flag");require(!post||r[0]==1,"Pre period after post period");double v=0;for(std::size_t j=0;j<d;++j)v+=p[j]*r[j+2];double g=r[1]-v;if(r[0]){post=true;++m;b+=g*g;c+=g;}else{++n;a+=g*g;}}
    require(n>0&&m>0,"Both time periods required");double u=std::sqrt(positive(a)/n),v=std::sqrt(b/m);return scalar({"PreRMSPE","PostRMSPE","PostPreRatio","MeanPostGap"},{u,v,v/u,c/m});
}
} }
