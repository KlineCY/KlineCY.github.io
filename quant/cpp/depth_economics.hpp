#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_economics {
using namespace evidence;
inline Report leontief_demand(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,2);require(x.size()==d,"Square technology matrix required");double tol=positive(p[0]);auto K=count(p[1],1,100000);require(double(K)*d*d<=5e7,"Iteration work limit");Mat a;Vec b;
    for(auto&r:x){Vec v(r.begin(),r.end()-1);for(double z:v)require(z>=0,"Negative technology coefficient");require(sum(v)<1&&r.back()>=0,"Row sum must be below one and demand nonnegative");a.push_back(v);b.push_back(r.back());}
    Vec y(d,0),r(d,0);double err=std::numeric_limits<double>::infinity();for(std::size_t k=0;k<K;++k){Vec z=mul(a,y);for(std::size_t j=0;j<d;++j)z[j]+=b[j];y=z;Vec m=mul(a,y);err=0;for(std::size_t j=0;j<d;++j){r[j]=y[j]-m[j]-b[j];err=std::max(err,std::abs(r[j]));}if(err<=tol)break;}require(err<=tol,"Input-output iteration did not converge");
    Report o{{"Sector","DirectDemand","TotalOutput","IndirectOutput","Residual"},{}};for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),b[j],y[j],y[j]-b[j],r[j]});return o;
}

inline Report fisher_price_index(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);double a=0,b=0,c=0,d=0;for(auto&r:x){positive(r[0]);positive(r[2]);require(r[1]>=0&&r[3]>=0,"Negative quantity");a+=r[0]*r[1];b+=r[2]*r[1];c+=r[0]*r[3];d+=r[2]*r[3];}positive(a);positive(b);positive(c);positive(d);double l=b/a,pq=d/c,f=std::sqrt(l*pq),v=d/a;return scalar({"Laspeyres","Paasche","Fisher","NominalValueRatio","QuantityFisher"},{l,pq,f,v,v/f});
}

inline Report shift_share_exposure(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);std::map<double,std::pair<double,double>> g;std::map<double,double> shock;std::set<std::pair<double,double>> keys;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]==std::floor(r[1])&&r[2]>=0&&keys.emplace(r[0],r[1]).second,"Invalid share key");auto z=shock.emplace(r[1],r[3]);require(z.second||z.first->second==r[3],"Industry shock differs by region");g[r[0]].first+=r[2];g[r[0]].second+=r[2]*r[3];}
    Report o{{"Region","ShareSum","Exposure"},{}};for(auto&z:g){require(std::abs(z.second.first-1)<1e-10,"Regional shares must sum to one");o.rows.push_back({z.first,z.second.first,z.second.second});}return o;
}

inline Report taylor_inertia(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,6);double r=p[0],pi=p[1],a=p[2],b=p[3],rho=p[4],i=p[5];require(rho>=0&&rho<1,"Invalid interest smoothing");Report o{{"Period","TargetRate","SmoothedRate","RealRateApprox"},{}};
    for(std::size_t j=0;j<x.size();++j){auto&z=x[j];double t=r+pi+a*(z[0]-pi)+b*z[1];i=rho*i+(1-rho)*t;o.rows.push_back({double(j+1),t,i,i-z[0]});}return o;
}
} }
