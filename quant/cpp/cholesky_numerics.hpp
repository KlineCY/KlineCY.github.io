#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline Matrix cholesky_factor(const Matrix& a) {
    check_covariance(a);
    int n=int(a.size());
    Matrix L(n,std::vector<double>(n,0.0));
    double scale=0;
    for(int i=0;i<n;i++)scale=std::max(scale,a[i][i]);
    require(scale>0,"Cholesky requires a positive definite matrix");
    for(int i=0;i<n;i++){
        for(int j=0;j<=i;j++){
            double s=a[i][j];
            for(int k=0;k<j;k++)s-=L[i][k]*L[j][k];
            if(i==j){
                require(s>scale*1e-13,"Cholesky needs positive definite input");
                L[i][j]=std::sqrt(s);
            }else L[i][j]=s/L[j][j];
        }
    }
    return L;
}
inline std::vector<double> solve_cholesky(const Matrix& a,
                                          const std::vector<double>& b){
    check_finite(b);
    auto L=cholesky_factor(a);int n=int(L.size());
    require(int(b.size())==n,"Cholesky solve rhs dimension");
    std::vector<double> y(n),x(n);
    for(int i=0;i<n;i++){
        double v=b[i];for(int j=0;j<i;j++)v-=L[i][j]*y[j];
        y[i]=v/L[i][i];
    }
    for(int i=n-1;i>=0;i--){
        double v=y[i];
        for(int j=i+1;j<n;j++)v-=L[j][i]*x[j];
        x[i]=v/L[i][i];
    }
    return x;
}
inline double log_determinant_positive_definite(const Matrix& a){
    auto L=cholesky_factor(a);
    double sum=0;for(int i=0;i<int(L.size());i++)sum+=std::log(L[i][i]);
    return 2*sum;
}
inline double multivariate_gaussian_log_density(
    const std::vector<double>& observation,
    const std::vector<double>& mu,const Matrix& covariance){
    check_finite(observation);check_finite(mu);
    require(observation.size()==mu.size()&&observation.size()==covariance.size(),
            "Gaussian dimensions mismatch");
    std::vector<double> difference(mu.size());
    for(int i=0;i<int(mu.size());i++)difference[i]=observation[i]-mu[i];
    auto solved=solve_cholesky(covariance,difference);
    double q=0;for(int i=0;i<int(mu.size());i++)q+=difference[i]*solved[i];
    return -0.5*(mu.size()*std::log(2*std::acos(-1.0))+
                 log_determinant_positive_definite(covariance)+q);
}
} // namespace kline
