#pragma once
#include "applied_common.hpp"
namespace applied_options {
Vec one_step_claim(const Vec& x){size(x,6);double s=x[0],u=x[1],d=x[2],r=x[3],hu=x[4],hd=x[5];check(s>0&&d>0&&r>0&&d<r*s&&r*s<u);double delta=(hu-hd)/(u-d),b=(hd-delta*d)/r,q=(r*s-d)/(u-d),v=delta*s+b;return {v,delta,b,q,v-(q*hu+(1-q)*hd)/r};}

Vec american_tree(const Vec& x){size(x,7);double s=x[0],k=x[1],u=x[2],d=x[3],r=x[4],cp=x[6];size_t n=integer(x[5],1,400);check(s>0&&k>0&&d>0&&d<r&&r<u&&(cp==1||cp==-1));double q=(r-d)/(u-d);auto payoff=[&](size_t i,size_t j){double p=s*std::pow(u,j)*std::pow(d,i-j);check(std::isfinite(p));return std::max(0.,cp*(p-k));};Vec a(n+1),e(n+1);for(size_t j=0;j<=n;++j)a[j]=e[j]=payoff(n,j);double count=0;for(size_t i=n;i-->0;){for(size_t j=0;j<=i;++j){double hold=(q*a[j+1]+(1-q)*a[j])/r,ex=payoff(i,j);count+=ex>hold+1e-10;a[j]=std::max(ex,hold);e[j]=(q*e[j+1]+(1-q)*e[j])/r;}}return {a[0],e[0],a[0]-e[0],count};}

Vec digital_vertical(const Vec& x){size(x,5);double a=x[0],b=x[1],c=x[2],d=x[3],df=x[4];check(a>0&&b>a&&c>=d&&d>=0&&df>0&&c-d<=df*(b-a)+1e-12);return {(c-d)/(b-a),(c-d)/(df*(b-a)),b-a};}

Vec uneven_butterfly(const Vec& x){size(x,7);double a=x[0],b=x[1],c=x[2],p=x[3],q=x[4],r=x[5],df=x[6];check(a>0&&a<b&&b<c&&p>=0&&q>=0&&r>=0&&df>0);double l=(q-p)/(b-a),h=(r-q)/(c-b),w=(b-a)/(c-a);return {l,h,h-l,2*(h-l)/((c-a)*df),(1-w)*p+w*r-q};}

Vec discrete_hedge(const Vec& x){check(x.size()>=6&&x.size()%2==0&&x[1]>0&&x[3]>0);double b=x[0]-x[4]*x[3],r=x[1];for(size_t i=4;i<x.size();i+=2){double p=x[i+1];check(p>0);b*=r;if(i+2<x.size())b-=(x[i+2]-x[i])*p;else{double w=b+x[i]*p;return {w,x[2],w-x[2],b};}}throw std::invalid_argument("missing terminal spot");}

Vec delta_vega_neutral(const Vec& x){size(x,6);check(x[4]!=0);double h=-x[1]/x[4],s=-x[0]-h*x[3];return {h,s,x[0]+h*x[3]+s,x[1]+h*x[4],x[2]+h*x[5]};}

Vec two_step_replication(const Vec& x){size(x,6);double s=x[0],k=x[1],u=x[2],d=x[3],r=x[4],cp=x[5];check(s>0&&k>0&&d>0&&d<r&&r<u&&(cp==1||cp==-1));auto p=[&](double v){return std::max(0.,cp*(v-k));};double a=p(s*u*u),b=p(s*u*d),c=p(s*d*d);double vu=one_step_claim({s*u,s*u*u,s*u*d,r,a,b})[0],vd=one_step_claim({s*d,s*d*u,s*d*d,r,b,c})[0];Vec root=one_step_claim({s,s*u,s*d,r,vu,vd});double q=root[3],direct=(q*q*a+2*q*(1-q)*b+sq(1-q)*c)/(r*r);return {root[0],root[1],root[2],vu,vd,root[0]-direct};}
}
