#pragma once
#include "cholesky_numerics.hpp"
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double squared_euclidean_distance(const std::vector<double>& a,
                                         const std::vector<double>& b){
    check_finite(a);check_finite(b);
    require(a.size()==b.size(),"Kernel feature mismatch");
    double sum=0;
    for(int i=0;i<int(a.size());i++)sum+=std::pow(a[i]-b[i],2);
    return sum;
}
inline double squared_exponential_kernel(const std::vector<double>& a,
                         const std::vector<double>& b,double lengthscale){
    require(std::isfinite(lengthscale)&&lengthscale>0,"Invalid RBF lengthscale");
    return std::exp(-0.5*squared_euclidean_distance(a,b)/
                    (lengthscale*lengthscale));
}
struct KernelRidgeFit{Matrix x;std::vector<double> alpha;double lengthscale=1;};
inline KernelRidgeFit fit_kernel_ridge(const Matrix& x,
              const std::vector<double>& y,double lambda,double lengthscale){
    check_feature_table(x,y);
    require(std::isfinite(lambda)&&lambda>0&&
            std::isfinite(lengthscale)&&lengthscale>0,
            "Kernel ridge requires positive regularization and bandwidth");
    int n=int(x.size());Matrix k(n,std::vector<double>(n));
    for(int i=0;i<n;i++)for(int j=0;j<=i;j++)
        k[i][j]=k[j][i]=squared_exponential_kernel(x[i],x[j],lengthscale);
    for(int i=0;i<n;i++)k[i][i]+=lambda;
    return {x,solve_cholesky(k,y),lengthscale};
}
inline double kernel_ridge_prediction(const KernelRidgeFit& fit,
                                      const std::vector<double>& features){
    check_finite(features);
    double prediction=0;
    for(int i=0;i<int(fit.x.size());i++)
        prediction+=fit.alpha[i]*squared_exponential_kernel(
            fit.x[i],features,fit.lengthscale);
    return prediction;
}
} // namespace kline
