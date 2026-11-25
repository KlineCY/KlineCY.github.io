#pragma once
#include "applied_common.hpp"
namespace applied_credit {
Vec hazard_curve(const Vec& x){pairs(x);double s=1;Vec surv,pd;for(size_t i=0;i<x.size();i+=2){check(x[i]>0&&x[i+1]>=0);double a=-std::expm1(-x[i]*x[i+1]),m=s*a;s-=m;surv.push_back(s);pd.push_back(m);}surv.insert(surv.end(),pd.begin(),pd.end());surv.push_back(1-s);return surv;}

Vec risky_zero(const Vec& x){size(x,5);double f=x[0],t=x[1],r=x[2],l=x[3],rec=x[4];check(f>0&&t>0&&l>=0);prob(rec);double a=r+l,z=std::exp(-a*t),i=a==0?t:-std::expm1(-a*t)/a,b=f*z,c=rec*f*l*i;return {b+c,b,c,std::exp(-l*t)};}

Vec cds_discrete(const Vec& x){check(x.size()>=4&&(x.size()-1)%3==0);prob(x[0]);double prev=1,a=0,l=0;for(size_t i=1;i<x.size();i+=3){double dt=x[i],df=x[i+1],s=x[i+2];check(dt>0&&df>0&&s>=0&&s<=prev);a+=dt*df*s;l+=df*(prev-s);prev=s;}check(a>0);l*=1-x[0];return {l/a,a,l};}

Vec net_collateral(const Vec& x){check(x.size()>=4);size_t m=integer(x[0],1,10000),n=integer(x[1],1,10000);double h=x[2];check(h>=0&&x.size()==3+m*n);double g=0,e=0,c=0,ne=0,coll=0;for(size_t i=0;i<m;++i){double v=0;for(size_t j=0;j<n;++j){double z=x[3+i*n+j];v+=z;g+=std::max(z,0.);}double p=std::max(v,0.),k=std::max(p-h,0.);e+=p;c+=p-k;coll+=k;ne+=std::max(-v,0.);}return {g/m,e/m,c/m,ne/m,coll/m};}

Vec cva_independent(const Vec& x){check(x.size()>=4&&(x.size()-1)%3==0);prob(x[0]);Vec out;double s=0,p=0;for(size_t i=1;i<x.size();i+=3){check(x[i]>0&&x[i+1]>=0);prob(x[i+2]);p+=x[i+2];double a=(1-x[0])*x[i]*x[i+1]*x[i+2];out.push_back(a);s+=a;}check(p<=1+1e-12);out.push_back(s);return out;}

Vec wrong_way(const Vec& x){check(x.size()>=4&&(x.size()-1)%3==0);prob(x[0]);double s=0,e=0,p=0,j=0;for(size_t i=1;i<x.size();i+=3){prob(x[i]);prob(x[i+2]);check(x[i+1]>=0);s+=x[i];e+=x[i]*x[i+1];p+=x[i]*x[i+2];j+=x[i]*x[i+1]*x[i+2];}check(std::abs(s-1)<1e-10);j*=1-x[0];double a=(1-x[0])*e*p;return {e,p,j,a,j-a};}

Vec credit_curve_pipeline(const Vec& x){check(x.size()>=5&&(x.size()-1)%4==0);size_t n=(x.size()-1)/4;Vec h,c{x[0]},v{x[0]};for(size_t i=1;i<x.size();i+=4)h.insert(h.end(),{x[i],x[i+1]});Vec s=hazard_curve(h);for(size_t j=0;j<n;++j){size_t i=1+4*j;c.insert(c.end(),{x[i],x[i+2],s[j]});v.insert(v.end(),{x[i+2],x[i+3],s[n+j]});}Vec cds=cds_discrete(c),cva=cva_independent(v);return {s.back(),cds[0],cva.back(),cva[0],s[n-1]};}
}
