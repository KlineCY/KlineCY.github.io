#pragma once
#include "householder_qr.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline QrFit weighted_qr_intercept(const Matrix& x,
             const std::vector<double>& y,const std::vector<double>& weights){
    check_feature_table(x,y);check_finite(weights);
    require(x.size()==weights.size(),"Weighted regression dimension mismatch");
    Matrix design=x;
    std::vector<double> response=y;
    for(int i=0;i<int(x.size());i++){
        require(weights[i]>0,"Weighted QR needs strictly positive weights");
        double root=std::sqrt(weights[i]);
        design[i].insert(design[i].begin(),1.0);
        for(double& value:design[i])value*=root;
        response[i]*=root;
    }
    return solve_householder_least_squares(design,response);
}
} // namespace kline
