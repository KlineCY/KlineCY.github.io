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
namespace fsq {
using Vec=std::vector<double>;
using Mat=std::vector<Vec>;
inline void require(bool ok){if(!ok)throw std::invalid_argument("input outside model domain");}
inline int count(double x,int lo,int hi){require(std::isfinite(x)&&x>=lo&&x<=hi&&std::floor(x)==x);return static_cast<int>(x);}
inline void probability(double x){require(x>=0&&x<=1);}
inline double sum(const Vec& x){return std::accumulate(x.begin(),x.end(),0.);}
inline void distribution(const Vec& x){require(!x.empty());for(double v:x)probability(v);require(std::abs(sum(x)-1)<1e-10);}
inline double sq(double x){return x*x;}
inline double logadd(double a,double b){if(a==-std::numeric_limits<double>::infinity())return b;if(b==-std::numeric_limits<double>::infinity())return a;double m=std::max(a,b);return m+std::log1p(std::exp(std::min(a,b)-m));}
inline double logp(double p){probability(p);return p==0?-std::numeric_limits<double>::infinity():std::log(p);}
inline double lse(const Vec& x){double a=-std::numeric_limits<double>::infinity();for(double b:x)a=logadd(a,b);return a;}
inline double normalcdf(double x){return .5*std::erfc(-x/std::sqrt(2.));}
}
