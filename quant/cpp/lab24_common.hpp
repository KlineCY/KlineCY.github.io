#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace kline { namespace lab24 {
using Vec=std::vector<double>;
using Mat=std::vector<Vec>;

inline void inputs(const Mat& x,const Vec& p,std::size_t cols,std::size_t params,
                   std::size_t min_rows=1) {
    require(x.size()>=min_rows&&x.size()<=250000,"Invalid number of observations");
    require(p.size()==params,"Incorrect scalar parameter count");
    for(double v:p)require(std::isfinite(v),"Nonfinite scalar parameter");
    for(const auto& row:x){
        require(row.size()==cols,"Unexpected CSV column count");
        for(double v:row)require(std::isfinite(v),"Nonfinite matrix entry");
    }
}
inline std::size_t integer(double x,std::size_t low,std::size_t high){
    require(std::isfinite(x)&&x>=double(low)&&x<=double(high)&&std::floor(x)==x,
            "Integer parameter outside allowed range");
    return std::size_t(x);
}
inline double dot(const Vec& a,const Vec& b){
    require(a.size()==b.size(),"Dot product dimension mismatch");
    return std::inner_product(a.begin(),a.end(),b.begin(),0.0);
}
inline Vec col(const Mat& x,std::size_t j){
    Vec out;out.reserve(x.size());
    for(const auto& row:x)out.push_back(row.at(j));
    return out;
}
inline double positive(double x){require(x>0&&std::isfinite(x),"Positive finite quantity required");return x;}
inline Vec checked(Vec v){check_finite(v);return v;}
inline double logistic(double x){
    if(x>=0)return 1/(1+std::exp(-x));
    double e=std::exp(x);return e/(1+e);
}
inline double normal_cdf(double x){return 0.5*std::erfc(-x/std::sqrt(2.0));}
inline double sum(const Vec& v){return std::accumulate(v.begin(),v.end(),0.0);}
} }
