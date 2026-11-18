#pragma once
#include "kernel_ridge.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double squared_mmd_rbf(const Matrix& a,const Matrix& b,double bandwidth){
    require(a.size()>=2&&b.size()>=2&&
            std::isfinite(bandwidth)&&bandwidth>0,
            "MMD requires two samples and a positive bandwidth");
    int n=int(a.size()),m=int(b.size());
    double aa=0,bb=0,ab=0;
    for(int i=0;i<n;i++)for(int j=0;j<n;j++)
        if(i!=j)aa+=squared_exponential_kernel(a[i],a[j],bandwidth)/
                      (n*double(n-1));
    for(int i=0;i<m;i++)for(int j=0;j<m;j++)
        if(i!=j)bb+=squared_exponential_kernel(b[i],b[j],bandwidth)/
                      (m*double(m-1));
    for(int i=0;i<n;i++)for(int j=0;j<m;j++)
        ab+=squared_exponential_kernel(a[i],b[j],bandwidth)/(n*double(m));
    return aa+bb-2*ab;
}
} // namespace kline
