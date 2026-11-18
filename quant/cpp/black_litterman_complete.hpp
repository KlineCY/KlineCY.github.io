#pragma once
#include "allocation.hpp"
#include "matrix_algorithms.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct FullBlackLitterman{std::vector<double> mean;Matrix covariance;};
inline FullBlackLitterman black_litterman_full(
    const std::vector<double>& equilibrium,const Matrix& covariance,
    const Matrix& views,const std::vector<double>& view_returns,
    const std::vector<double>& view_variances,double tau){
    check_finite(equilibrium);require_positive_semidefinite(covariance);
    require(equilibrium.size()==covariance.size()&&
            views.size()==view_returns.size()&&
            views.size()==view_variances.size()&&!views.empty()&&
            std::isfinite(tau)&&tau>0,"Invalid Black-Litterman dimensions");
    int n=int(equilibrium.size()),k=int(views.size());
    check_finite(view_returns);check_finite(view_variances);
    Matrix omega(k,std::vector<double>(k,0));
    for(int a=0;a<k;a++){
        check_finite(views[a]);require(int(views[a].size())==n&&
                                     view_variances[a]>0,"Invalid return view");
        omega[a][a]=view_variances[a];
    }
    Matrix scaled=covariance;
    for(auto& row:scaled)for(double& value:row)value*=tau;
    Matrix pt=matrix_transpose(views);
    Matrix intermediate=matrix_multiply(views,matrix_multiply(scaled,pt));
    for(int i=0;i<k;i++)intermediate[i][i]+=omega[i][i];
    Matrix solved_matrix(k,std::vector<double>(k,0));
    for(int j=0;j<k;j++){
        std::vector<double> e(k,0);e[j]=1;
        auto solution=solve_linear_system(intermediate,e);
        for(int i=0;i<k;i++)solved_matrix[i][j]=solution[i];
    }
    std::vector<double> deviations(k);
    for(int i=0;i<k;i++){
        double predicted=0;
        for(int j=0;j<n;j++)predicted+=views[i][j]*equilibrium[j];
        deviations[i]=view_returns[i]-predicted;
    }
    Matrix gain=matrix_multiply(matrix_multiply(scaled,pt),solved_matrix);
    FullBlackLitterman result;
    result.mean=equilibrium;
    for(int i=0;i<n;i++)
        for(int j=0;j<k;j++)result.mean[i]+=gain[i][j]*deviations[j];
    auto correction=matrix_multiply(gain,matrix_multiply(views,scaled));
    result.covariance=covariance;
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            result.covariance[i][j]+=scaled[i][j]-correction[i][j];
    return result;
}
} // namespace kline
