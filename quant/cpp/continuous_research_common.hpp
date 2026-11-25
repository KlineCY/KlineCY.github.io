#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace crq {
using Vec=std::vector<double>;
using Mat=std::vector<Vec>;
using V2=std::array<double,2>;
using M2=std::array<std::array<double,2>,2>;
inline void check(bool b){if(!b)throw std::invalid_argument("outside model input domain");}
inline int integer(double x,int l,int h){check(std::isfinite(x)&&x>=l&&x<=h&&std::floor(x)==x);return static_cast<int>(x);}
inline double sq(double x){return x*x;}
inline double dot(V2 a,V2 b){return a[0]*b[0]+a[1]*b[1];}
inline V2 add(V2 a,V2 b){return {a[0]+b[0],a[1]+b[1]};}
inline V2 sub(V2 a,V2 b){return {a[0]-b[0],a[1]-b[1]};}
inline V2 scale(V2 a,double s){return {s*a[0],s*a[1]};}
inline M2 sum(M2 a,M2 b,double factor=1){for(int i=0;i<2;++i)for(int j=0;j<2;++j)a[i][j]+=factor*b[i][j];return a;}
inline M2 trans(M2 a){std::swap(a[0][1],a[1][0]);return a;}
inline V2 mult(M2 a,V2 b){return {dot(a[0],b),dot(a[1],b)};}
inline M2 mult(M2 a,M2 b){M2 c{};for(int i=0;i<2;++i)for(int j=0;j<2;++j)for(int k=0;k<2;++k)c[i][j]+=a[i][k]*b[k][j];return c;}
inline M2 outer(V2 a,V2 b){return {{{a[0]*b[0],a[0]*b[1]},{a[1]*b[0],a[1]*b[1]}}};}
inline M2 sym(M2 a){a[0][1]=a[1][0]=(a[0][1]+a[1][0])/2;return a;}
inline M2 covariance(double a,double b,double c){return {{{a,b},{b,c}}};}
inline M2 eye(){return covariance(1,0,1);}
inline bool spd(M2 a){return a[0][0]>0&&a[1][1]>0&&a[0][0]*a[1][1]>sq(a[0][1]);}
inline M2 inv(M2 a){double d=a[0][0]*a[1][1]-a[0][1]*a[1][0];check(d>0);return {{{a[1][1]/d,-a[0][1]/d},{-a[1][0]/d,a[0][0]/d}}};}
inline M2 cholesky(M2 a){check(spd(a));double l=std::sqrt(a[0][0]),m=a[1][0]/l,n=std::sqrt(a[1][1]-m*m);return {{{l,0},{m,n}}};}
inline void append(Vec& out,V2 m,M2 p){out.insert(out.end(),{m[0],m[1],p[0][0],p[0][1],p[1][1]});}
inline double cdf(double x){return .5*std::erfc(-x/std::sqrt(2.));}
inline double pdf(double x){return std::exp(-.5*x*x)/std::sqrt(2*std::acos(-1.));}
inline double bs(double s,double k,double r,double q,double vol,double t,double cp){
    check(s>0&&k>0&&vol>0&&t>=0);
    if(t==0)return std::max(cp*(s-k),0.);
    double st=vol*std::sqrt(t),d1=(std::log(s/k)+(r-q+.5*vol*vol)*t)/st,d2=d1-st;
    return cp*(s*std::exp(-q*t)*cdf(cp*d1)-k*std::exp(-r*t)*cdf(cp*d2));
}
inline Vec tri_solve(Vec diag,const Vec& lower,const Vec& upper,Vec rhs){
    int n=diag.size();check(n>=1&&lower.size()==size_t(n-1)&&upper.size()==size_t(n-1)&&rhs.size()==size_t(n));
    for(int i=1;i<n;++i){check(std::abs(diag[i-1])>1e-15);double a=lower[i-1]/diag[i-1];diag[i]-=a*upper[i-1];rhs[i]-=a*rhs[i-1];}
    check(std::abs(diag.back())>1e-15);rhs.back()/=diag.back();
    for(int i=n-2;i>=0;--i){check(std::abs(diag[i])>1e-15);rhs[i]=(rhs[i]-upper[i]*rhs[i+1])/diag[i];}
    return rhs;
}
inline Vec tri_mult(const Vec& d,const Vec& l,const Vec& u,const Vec& x){
    Vec y(x.size());for(size_t i=0;i<x.size();++i){y[i]=d[i]*x[i];if(i)y[i]+=l[i-1]*x[i-1];if(i+1<x.size())y[i]+=u[i]*x[i+1];}return y;
}
inline double residual(const Vec& a,const Vec& b){check(a.size()==b.size());double z=0;for(size_t i=0;i<a.size();++i)z=std::max(z,std::abs(a[i]-b[i]));return z;}
inline constexpr std::array<double,5> gh_z{-2.8569700138728056,-1.355626179974266,0,1.355626179974266,2.8569700138728056};
inline constexpr std::array<double,5> gh_w{.0112574113277207,.2220759220056126,.5333333333333333,.2220759220056126,.0112574113277207};
}
