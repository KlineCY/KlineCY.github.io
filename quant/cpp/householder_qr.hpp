#pragma once
#include "learning.hpp"
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
struct QrFit { std::vector<double> coefficients; double residual_sum_squares=0; };
inline QrFit solve_householder_least_squares(Matrix x,std::vector<double> y){
    require(!x.empty()&&!x[0].empty()&&x.size()==y.size(),"QR input dimensions");
    check_finite(y);int m=int(x.size()),n=int(x[0].size());
    require(m>=n,"QR requires at least as many samples as predictors");
    for(const auto& row:x){require(int(row.size())==n,"QR ragged design");check_finite(row);}
    for(int j=0;j<n;j++){
        double norm=0,scale=0;
        for(int i=j;i<m;i++)scale=std::max(scale,std::fabs(x[i][j]));
        require(scale>0,"Rank-deficient design");
        for(int i=j;i<m;i++)norm+=std::pow(x[i][j]/scale,2);
        norm=scale*std::sqrt(norm);
        double alpha=x[j][j]>=0?-norm:norm;
        std::vector<double> v(m-j);
        v[0]=x[j][j]-alpha;
        for(int i=j+1;i<m;i++)v[i-j]=x[i][j];
        double squared=std::inner_product(v.begin(),v.end(),v.begin(),0.0);
        require(squared>0,"Degenerate Householder reflector");
        double factor=2/squared;
        for(int k=j;k<n;k++){
            double dot=0;for(int i=j;i<m;i++)dot+=v[i-j]*x[i][k];
            for(int i=j;i<m;i++)x[i][k]-=factor*v[i-j]*dot;
        }
        double ydot=0;for(int i=j;i<m;i++)ydot+=v[i-j]*y[i];
        for(int i=j;i<m;i++)y[i]-=factor*v[i-j]*ydot;
        require(std::fabs(x[j][j])>1e-12*scale,"QR rank deficiency");
    }
    QrFit fit;fit.coefficients.assign(n,0);
    for(int i=n-1;i>=0;i--){
        double r=y[i];
        for(int j=i+1;j<n;j++)r-=x[i][j]*fit.coefficients[j];
        fit.coefficients[i]=r/x[i][i];
    }
    for(int i=n;i<m;i++)fit.residual_sum_squares+=y[i]*y[i];
    return fit;
}
inline QrFit householder_ols_intercept(const Matrix& x,const std::vector<double>& y){
    check_feature_table(x,y);
    Matrix with_constant=x;
    for(auto& row:with_constant)row.insert(row.begin(),1.0);
    return solve_householder_least_squares(with_constant,y);
}
} // namespace kline
