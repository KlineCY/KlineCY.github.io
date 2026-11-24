#pragma once
#include "numerical_lab_common.hpp"
#include <functional>
#include <set>
#include <tuple>
namespace kline { namespace evidence {
using namespace numerical_lab;
inline Mat moment(const Mat& x,bool centered){
    auto d=validate(x,2,1,32);Vec mu(d,0);Mat a(d,Vec(d,0));
    if(centered)for(const auto& r:x)for(std::size_t j=0;j<d;++j)mu[j]+=r[j]/x.size();
    for(const auto& r:x)for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<d;++k)
        a[j][k]+=(r[j]-mu[j])*(r[k]-mu[k])/x.size();
    return a;
}
inline double distance(const Vec& a,const Vec& b){double d=0;for(std::size_t j=0;j<a.size();++j)d=std::hypot(d,a[j]-b[j]);return d;}
inline Vec means(const Mat& x){Vec m(x[0].size());for(std::size_t j=0;j<m.size();++j)m[j]=mean(column(x,j));return m;}
inline std::vector<std::size_t> order(const Vec& x){std::vector<std::size_t> ix(x.size());std::iota(ix.begin(),ix.end(),0);std::stable_sort(ix.begin(),ix.end(),[&](auto a,auto b){return x[a]<x[b];});return ix;}
inline std::size_t bits(std::uint64_t n){std::size_t k=0;while(n){n&=n-1;++k;}return k;}
inline Report scalar(const std::vector<std::string>& h,const Vec& x){return {h,{x}};}
} }
