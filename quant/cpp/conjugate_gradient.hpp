#pragma once
#include "cholesky_numerics.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
struct CgSolution {std::vector<double> x;int iterations=0;double residual=0;};
inline CgSolution conjugate_gradient(const Matrix& a,const std::vector<double>& b,
                   double tolerance=1e-10,int max_iterations=2000){
    check_finite(b);
    require(a.size()==b.size()&&std::isfinite(tolerance)&&tolerance>0&&
            max_iterations>0,"CG solver parameters");
    cholesky_factor(a);
    int n=int(b.size());
    std::vector<double> x(n,0),r=b,p=b;
    double rr=std::inner_product(r.begin(),r.end(),r.begin(),0.0);
    double target=tolerance*tolerance*
        std::max(1.0,std::inner_product(b.begin(),b.end(),b.begin(),0.0));
    if(rr<=target)return {x,0,std::sqrt(rr)};
    for(int k=0;k<max_iterations;k++){
        std::vector<double> ap(n,0);
        for(int i=0;i<n;i++)for(int j=0;j<n;j++)ap[i]+=a[i][j]*p[j];
        double curvature=std::inner_product(p.begin(),p.end(),ap.begin(),0.0);
        require(curvature>0,"CG lost positive curvature");
        double alpha=rr/curvature;
        for(int i=0;i<n;i++){x[i]+=alpha*p[i];r[i]-=alpha*ap[i];}
        double next=std::inner_product(r.begin(),r.end(),r.begin(),0.0);
        if(next<=target)return {x,k+1,std::sqrt(next)};
        double beta=next/rr;
        for(int i=0;i<n;i++)p[i]=r[i]+beta*p[i];
        rr=next;
    }
    throw std::runtime_error("Conjugate gradient did not converge");
}
} // namespace kline
