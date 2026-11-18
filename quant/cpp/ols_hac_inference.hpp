#pragma once
#include "householder_qr.hpp"
#include "matrix_algorithms.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline Matrix ols_newey_west_covariance(const Matrix& x,
            const std::vector<double>& residuals,int lags){
    require(!x.empty()&&x.size()==residuals.size()&&lags>=0&&
            lags<int(x.size()),"HAC dimensions or lags");
    check_finite(residuals);int n=int(x.size()),p=int(x[0].size())+1;
    require(n>p,"HAC needs residual degrees of freedom");
    Matrix design(n,std::vector<double>(p,1));
    for(int t=0;t<n;t++){
        require(int(x[t].size())==p-1,"Ragged HAC features");check_finite(x[t]);
        for(int j=1;j<p;j++)design[t][j]=x[t][j-1];
    }
    Matrix xx(p,std::vector<double>(p,0)),s(p,std::vector<double>(p,0));
    for(int t=0;t<n;t++)for(int i=0;i<p;i++)for(int j=0;j<p;j++){
        xx[i][j]+=design[t][i]*design[t][j];
        s[i][j]+=residuals[t]*residuals[t]*design[t][i]*design[t][j];
    }
    for(int lag=1;lag<=lags;lag++){
        double weight=1.0-double(lag)/(lags+1);
        for(int t=lag;t<n;t++)
            for(int i=0;i<p;i++)for(int j=0;j<p;j++)
                s[i][j]+=weight*residuals[t]*residuals[t-lag]*
                   (design[t][i]*design[t-lag][j]+design[t-lag][i]*design[t][j]);
    }
    auto xxinv=inverse_matrix(xx);
    return matrix_multiply(matrix_multiply(xxinv,s),xxinv);
}
inline std::vector<double> ols_hac_standard_errors(const Matrix& x,
            const std::vector<double>& residuals,int lags){
    auto covariance=ols_newey_west_covariance(x,residuals,lags);
    std::vector<double> out;
    for(int i=0;i<int(covariance.size());i++){
        require(covariance[i][i]>=-1e-12,"Negative HAC variance");
        out.push_back(std::sqrt(std::max(0.0,covariance[i][i])));
    }
    return out;
}
} // namespace kline
