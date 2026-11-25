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
using Vec=std::vector<double>;
inline void check(bool b){if(!b)throw std::invalid_argument("invalid model input");}
inline double sq(double x){return x*x;}
inline void size(const Vec& x,size_t n){check(x.size()==n);}
inline void pairs(const Vec& x){check(!x.empty()&&x.size()%2==0);}
inline void prob(double x){check(x>=0&&x<=1);}
inline void binary(double x){check(x==0||x==1);}
inline size_t integer(double x,size_t l,size_t h){check(std::isfinite(x)&&x>=double(l)&&x<=double(h)&&std::floor(x)==x);return static_cast<size_t>(x);}
inline double mean(const Vec& x){check(!x.empty());return std::accumulate(x.begin(),x.end(),0.)/x.size();}
inline double choose(int n,int k){if(k<0||k>n||n<0)return 0;double v=1;k=std::min(k,n-k);for(int i=1;i<=k;++i)v*=double(n-k+i)/i;return v;}
