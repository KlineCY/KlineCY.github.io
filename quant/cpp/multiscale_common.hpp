#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
namespace msq {
using Vec=std::vector<double>;
using Mat=std::vector<Vec>;
inline void require(bool x){if(!x)throw std::invalid_argument("input outside model domain");}
inline int integer(double x,int lo,int hi){require(std::isfinite(x)&&x>=lo&&x<=hi&&std::floor(x)==x);return int(x);}
inline double sq(double x){return x*x;}
inline double sum(const Vec& x){return std::accumulate(x.begin(),x.end(),0.);}
inline double avg(const Vec& x){require(!x.empty());return sum(x)/x.size();}
inline double var(const Vec& x){require(x.size()>=2);double m=avg(x),v=0;for(double a:x)v+=sq(a-m);return v/(x.size()-1);}
inline double cov(const Vec& x,const Vec& y){require(x.size()==y.size()&&x.size()>=2);double a=avg(x),b=avg(y),v=0;for(size_t i=0;i<x.size();++i)v+=(x[i]-a)*(y[i]-b);return v/(x.size()-1);}
inline double cdf(double x){return .5*std::erfc(-x/std::sqrt(2.));}
inline double pdf(double x){return std::exp(-x*x/2)/std::sqrt(2*std::acos(-1.));}
inline double normalmass(double a,double b){return a>=0?cdf(-a)-cdf(-b):cdf(b)-cdf(a);}
inline void positive(double x){require(x>0&&std::isfinite(x));}
inline void prob(double x){require(x>=0&&x<=1);}
inline Vec solve(Mat a,Vec b){
    int n=b.size();require(a.size()==b.size()&&n>0);
    double scale=0;for(const auto& r:a){require(r.size()==b.size());for(double z:r)scale=std::max(scale,std::abs(z));}
    positive(scale);
    for(int j=0;j<n;++j){int p=j;for(int i=j+1;i<n;++i)if(std::abs(a[i][j])>std::abs(a[p][j]))p=i;
        require(std::abs(a[p][j])>1e-12*scale);std::swap(a[p],a[j]);std::swap(b[p],b[j]);
        for(int i=j+1;i<n;++i){double z=a[i][j]/a[j][j];for(int k=j;k<n;++k)a[i][k]-=z*a[j][k];b[i]-=z*b[j];}
    }
    Vec x(n);for(int i=n-1;i>=0;--i){double y=b[i];for(int j=i+1;j<n;++j)y-=a[i][j]*x[j];x[i]=y/a[i][i];}return x;
}
inline double dot(const Vec& a,const Vec& b){require(a.size()==b.size());return std::inner_product(a.begin(),a.end(),b.begin(),0.);}
}
