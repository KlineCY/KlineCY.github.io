#pragma once
#include "applied_common.hpp"
namespace applied_inference {
Vec pit_ks(const Vec& x){check(!x.empty());Vec a=x;for(double z:a)prob(z);std::sort(a.begin(),a.end());double p=0,m=0,n=a.size();for(size_t i=0;i<a.size();++i){p=std::max(p,(i+1)/n-a[i]);m=std::max(m,a[i]-i/n);}return {p,m,std::max(p,m)};}

Vec pit_cvm(const Vec& x){check(!x.empty());Vec a=x;for(double z:a)prob(z);std::sort(a.begin(),a.end());double n=a.size(),g=1/(12*n),v=g;for(size_t i=0;i<a.size();++i)v+=sq(a[i]-(2*i+1)/(2*n));return {v,g};}

Vec pit_ad(const Vec& x){check(!x.empty());Vec a=x;for(double z:a)check(z>0&&z<1);std::sort(a.begin(),a.end());double n=a.size(),s=0;for(size_t i=0;i<a.size();++i)s+=(2*i+1)*(std::log(a[i])+std::log1p(-a[a.size()-1-i]));return {-n-s/n};}

Vec runs_exact(const Vec& x){check(x.size()>=2&&x.size()<=60);int a=0,b=0,r=1;for(size_t i=0;i<x.size();++i){check(x[i]==1||x[i]==-1);a+=x[i]==1;b+=x[i]==-1;if(i)r+=x[i]!=x[i-1];}check(a>0&&b>0);double den=choose(a+b,a),lo=0,hi=0;for(int j=2;j<=a+b;++j){int k=j/2;double ways=j%2?choose(a-1,k)*choose(b-1,k-1)+choose(a-1,k-1)*choose(b-1,k):2*choose(a-1,k-1)*choose(b-1,k-1);if(j<=r)lo+=ways/den;if(j>=r)hi+=ways/den;}return {double(r),1+2.*a*b/(a+b),lo,hi,std::min(1.,2*std::min(lo,hi))};}

Vec circular_boot_var(const Vec& x){check(x.size()>=3);size_t n=x.size()-1,b=integer(x[0],1,n);check(n%b==0);Vec s(x.begin()+1,x.end());double avg=mean(s),v=0;for(size_t i=0;i<n;++i){double t=0;for(size_t j=0;j<b;++j)t+=s[(i+j)%n];v+=sq(t/b-avg);}v/=n;return {avg,v,v/(n/b)};}

Vec ljung_box(const Vec& x){check(x.size()>=4);size_t n=x.size()-1,h=integer(x[0],1,n-1);Vec y(x.begin()+1,x.end());check(*std::min_element(y.begin(),y.end())<*std::max_element(y.begin(),y.end()));long double m=std::accumulate(y.begin(),y.end(),0.L)/n,den=0,q=0,first=0,last=0;for(double z:y){long double d=z-m;den+=d*d;}check(den>0);for(size_t k=1;k<=h;++k){long double r=0;for(size_t j=k;j<n;++j)r+=(y[j]-m)*(y[j-k]-m);r/=den;if(k==1)first=r;last=r;q+=static_cast<long double>(n)*(n+2)*r*r/(n-k);}return {double(q),double(first),double(last)};}

Vec pit_joint(const Vec& x){check(x.size()>=3);Vec y{1};y.insert(y.end(),x.begin(),x.end());return {pit_ks(x)[2],pit_cvm(x)[0],pit_ad(x)[0],ljung_box(y)[0]};}
}
