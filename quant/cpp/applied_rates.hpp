#pragma once
#include "applied_common.hpp"
namespace applied_rates {
Vec continuous_duration(const Vec& x){check(x.size()>=3&&x.size()%2==1);double p=0,d=0,c=0,last=0;for(size_t i=1;i<x.size();i+=2){double t=x[i],f=x[i+1];check(t>last&&f>0);last=t;double v=f*std::exp(-x[0]*t);p+=v;d+=t*v;c+=t*t*v;}check(p>0);return {p,d/p,c/p,d};}

Vec par_coupon(const Vec& x){pairs(x);double a=0;for(size_t i=0;i<x.size();i+=2){check(x[i]>0&&x[i+1]>0);a+=x[i]*x[i+1];}double dn=x.back(),c=(1-dn)/a;return {c,a,dn+c*a};}

Vec key_rate_pv(const Vec& x){check(!x.empty()&&x.size()%3==0);Vec out{0};double last=0;for(size_t i=0;i<x.size();i+=3){check(x[i]>last&&x[i+1]>0);last=x[i];double v=x[i+1]*std::exp(-x[i]*x[i+2]);out[0]+=v;out.push_back(x[i]*v);}return out;}

Vec yield_bisect(const Vec& x){check(x.size()>=3&&x.size()%2==1&&x[0]>0);Vec a=x;double target=x[0],lo=-2,hi=2;a[0]=lo;check(continuous_duration(a)[0]>=target);a[0]=hi;check(continuous_duration(a)[0]<=target);for(int i=0;i<100;++i){a[0]=(lo+hi)/2;if(continuous_duration(a)[0]>target)lo=a[0];else hi=a[0];}a[0]=(lo+hi)/2;double p=continuous_duration(a)[0];return {a[0],p,p-target};}

Vec forward_discount(const Vec& x){size(x,4);check(x[0]>=0&&x[1]>x[0]&&x[2]>0&&x[3]>0);double d=x[3]/x[2];return {d,-std::log(d)/(x[1]-x[0]),1/d};}

Vec two_zero_immunize(const Vec& x){size(x,5);double t=x[0],u=x[1],T=x[2],L=x[3],y=x[4];check(t>0&&u>t&&T>=t&&T<=u&&L>0);double pv=L*std::exp(-y*T),a=pv*(u-T)/(u-t),b=pv*(T-t)/(u-t);return {a*std::exp(y*t),b*std::exp(y*u),a+b,pv,a*t*t+b*u*u-pv*T*T};}

Vec curve_shock(const Vec& x){check(x.size()>=4&&x.size()%2==0);Vec a{x[0]};a.insert(a.end(),x.begin()+2,x.end());Vec p=continuous_duration(a);a[0]+=x[1];double next=continuous_duration(a)[0],l=-p[3]*x[1],q=l+.5*p[0]*p[2]*sq(x[1]);return {p[0],next,l,q,next-p[0]-q};}
}
