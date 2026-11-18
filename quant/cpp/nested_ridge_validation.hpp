#pragma once
#include "learning.hpp"
#include <limits>
#include <vector>
namespace kline {
struct RidgeSelection{
    double penalty=0,validation_mse=0;
    RidgeModel fitted;
};
inline RidgeSelection select_ridge_chronologically(
    const Matrix& x,const std::vector<double>& y,
    const std::vector<double>& penalties,int train_end,int validation_end){
    check_feature_table(x,y);check_finite(penalties);
    require(train_end>=4&&validation_end>train_end&&
            validation_end<=int(x.size())&&penalties.size()>=2,
            "Invalid chronological ridge validation");
    Matrix train(x.begin(),x.begin()+train_end);
    std::vector<double> labels(y.begin(),y.begin()+train_end);
    RidgeSelection best;best.validation_mse=
        std::numeric_limits<double>::infinity();
    for(double penalty:penalties){
        require(penalty>0,"Nested ridge requires positive penalties");
        auto model=fit_ridge(train,labels,penalty);
        double squared=0;
        for(int t=train_end;t<validation_end;t++)
            squared+=std::pow(y[t]-ridge_predict(model,x[t]),2);
        double mse=squared/(validation_end-train_end);
        if(mse<best.validation_mse){
            best.penalty=penalty;best.validation_mse=mse;
        }
    }
    Matrix extended(x.begin(),x.begin()+validation_end);
    std::vector<double> extended_y(y.begin(),y.begin()+validation_end);
    best.fitted=fit_ridge(extended,extended_y,best.penalty);
    return best;
}
} // namespace kline
