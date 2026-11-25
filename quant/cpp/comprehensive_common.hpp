#pragma once
#include "system_research_common.hpp"
#include <map>
#include <set>
#include <deque>
#include <tuple>
#include <random>
namespace kline { namespace comprehensive {
using namespace system_research;
inline double logsumexp(const Vec& x){
    require(!x.empty(),"Empty log-sum-exp");
    double m=*std::max_element(x.begin(),x.end()),s=0;
    for(double v:x)s+=std::exp(v-m);
    return m+std::log(positive(s));
}
inline double logdet_spd(const Mat& A){
    auto n=A.size();Mat L(n,Vec(n,0));double z=0;
    for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<=i;++j){
        double a=A[i][j];for(std::size_t k=0;k<j;++k)a-=L[i][k]*L[j][k];
        L[i][j]=i==j?std::sqrt(positive(a)):a/L[j][j];
        if(i==j)z+=2*std::log(L[i][j]);
    }
    return z;
}
} }
