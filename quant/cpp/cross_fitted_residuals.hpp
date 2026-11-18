#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct CrossFitResidual{std::vector<double> labels,model_residual;};
inline CrossFitResidual contiguous_crossfit_ridge(
    const Matrix& x,const std::vector<double>& y,int folds,double ridge){
    check_feature_table(x,y);
    require(folds>=2&&int(x.size())>=folds*3&&ridge>0,
            "Invalid cross-fitting groups");
    int n=int(y.size());
    CrossFitResidual result;result.labels=y;
    result.model_residual.resize(n);
    for(int k=0;k<folds;k++){
        int begin=k*n/folds,end=(k+1)*n/folds;
        Matrix train;
        std::vector<double> target;
        for(int i=0;i<n;i++)
            if(i<begin||i>=end){train.push_back(x[i]);target.push_back(y[i]);}
        auto model=fit_ridge(train,target,ridge);
        for(int i=begin;i<end;i++)
            result.model_residual[i]=y[i]-ridge_predict(model,x[i]);
    }
    return result;
}
} // namespace kline
