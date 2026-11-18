#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline Matrix matrix_transpose(const Matrix& a){
    require(!a.empty()&&!a.front().empty(),"Empty matrix");
    size_t cols=a.front().size();Matrix b(cols,std::vector<double>(a.size()));
    for(size_t i=0;i<a.size();i++){
        require(a[i].size()==cols,"Ragged matrix");check_finite(a[i]);
        for(size_t j=0;j<cols;j++)b[j][i]=a[i][j];
    }
    return b;
}
inline Matrix matrix_multiply(const Matrix& a,const Matrix& b){
    require(!a.empty()&&!b.empty()&&!a.front().empty()&&
            a.front().size()==b.size(),"Matrix multiplication shape");
    size_t m=a.size(),n=b.size(),p=b.front().size();
    require(p>0,"Matrix multiplication empty columns");
    Matrix c(m,std::vector<double>(p,0));
    for(const auto& row:a){require(row.size()==n,"Ragged left matrix");check_finite(row);}
    for(const auto& row:b){require(row.size()==p,"Ragged right matrix");check_finite(row);}
    for(size_t i=0;i<m;i++)
        for(size_t k=0;k<n;k++)
            for(size_t j=0;j<p;j++)c[i][j]+=a[i][k]*b[k][j];
    return c;
}
inline double determinant(Matrix a){
    require(!a.empty(),"Empty determinant");
    size_t n=a.size();for(const auto& row:a){
        require(row.size()==n,"Determinant needs square matrix");check_finite(row);
    }
    double result=1;
    for(size_t k=0;k<n;k++){
        size_t pivot=k;
        for(size_t i=k+1;i<n;i++)
            if(std::fabs(a[i][k])>std::fabs(a[pivot][k]))pivot=i;
        if(std::fabs(a[pivot][k])<1e-14)return 0;
        if(pivot!=k){std::swap(a[pivot],a[k]);result=-result;}
        double diag=a[k][k];result*=diag;
        for(size_t i=k+1;i<n;i++){
            double t=a[i][k]/diag;
            for(size_t j=k+1;j<n;j++)a[i][j]-=t*a[k][j];
        }
    }
    return result;
}
inline Matrix inverse_matrix(const Matrix& a){
    require(!a.empty(),"Empty inverse");
    size_t n=a.size();Matrix inverse(n,std::vector<double>(n));
    for(size_t j=0;j<n;j++){
        std::vector<double> unit(n,0);unit[j]=1;
        auto column=solve_linear_system(a,unit);
        for(size_t i=0;i<n;i++)inverse[i][j]=column[i];
    }
    return inverse;
}
inline double mahalanobis_squared(const std::vector<double>& x,
                                  const std::vector<double>& center,
                                  const Matrix& covariance){
    check_finite(x);check_finite(center);
    require(x.size()==center.size()&&x.size()==covariance.size(),
            "Mahalanobis dimensions");
    std::vector<double> d(x.size());
    for(size_t i=0;i<x.size();i++)d[i]=x[i]-center[i];
    auto solution=solve_linear_system(covariance,d);
    double result=std::inner_product(d.begin(),d.end(),solution.begin(),0.0);
    require(result>=-1e-10,"Mahalanobis requires positive-definite covariance");
    return std::max(0.0,result);
}
} // namespace kline
