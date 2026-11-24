#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_distribution {
using namespace evidence;
inline Report discrete_convolution(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);auto n=x.size();require(double(n)*n<=2e7,"Convolution work limit");
    double a=0,b=0;for(auto&r:x){require(r[0]>=0&&r[1]>=0,"Negative probability");a+=r[0];b+=r[1];}
    require(std::abs(a-1)<1e-10&&std::abs(b-1)<1e-10,"Probability sums");
    Vec v(2*n-1,0);for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)v[i+j]+=x[i][0]*x[j][1];
    Report o{{"Loss","Probability","Cumulative"},{}};double s=0;
    for(std::size_t i=0;i<v.size();++i){s+=v[i];o.rows.push_back({double(i),v[i],s});}return o;
}

inline Report latent_binomial(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);auto n=count(p[0],1,10000);require(double(n+1)*x.size()<=2e7,"Mixture work limit");
    double w=0;for(auto&r:x){require(r[0]>=0&&r[0]<=1&&r[1]>=0,"Invalid mixture");w+=r[1];}require(std::abs(w-1)<1e-10,"Probability sum");
    Report o{{"Defaults","Probability","Cumulative"},{}};double s=0;
    for(std::size_t k=0;k<=n;++k){
        double v=0;for(auto&r:x){double q=r[0],a=0;if(q==0)a=double(k==0);else if(q==1)a=double(k==n);else a=std::exp(std::lgamma(n+1.)-std::lgamma(k+1.)-std::lgamma(n-k+1.)+k*std::log(q)+(n-k)*std::log1p(-q));v+=r[1]*a;}
        s+=v;o.rows.push_back({double(k),v,s});
    }return o;
}

inline Report exponential_tilt(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);
    double s=0,m=-std::numeric_limits<double>::infinity();Vec logw;
    for(auto&r:x){require(r[1]>=0,"Negative base probability");s+=r[1];double v=r[1]>0?std::log(r[1])+p[0]*r[0]:-std::numeric_limits<double>::infinity();logw.push_back(v);m=std::max(m,v);}
    require(std::abs(s-1)<1e-10,"Probability sum");double z=0;for(double v:logw)z+=std::exp(v-m);double l=m+std::log(z),mu=0;Vec q;
    for(std::size_t i=0;i<x.size();++i){q.push_back(std::exp(logw[i]-m)/z);mu+=q.back()*x[i][0];}
    Report o{{"State","TiltProbability","LogNormalizer","TiltMean","KL"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({x[i][0],q[i],l,mu,p[0]*mu-l});return o;
}

inline Report randomized_pit(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);Report o{{"CDFLeft","CDFRight","PIT"},{}};
    for(auto&r:x){require(r[0]>=0&&r[1]<=1&&r[0]<=r[1]&&r[2]>=0&&r[2]<=1,"Invalid CDF or uniform");o.rows.push_back({r[0],r[1],r[0]+r[2]*(r[1]-r[0])});}return o;
}
} }
