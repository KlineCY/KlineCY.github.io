#pragma once
#include "tree_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct BoostedStumps{
    double baseline=0;
    double learning_rate=0.1;
    std::vector<RegressionStump> stumps;
};
inline BoostedStumps gradient_boost_stumps(
    const Matrix& x,const std::vector<double>& y,int rounds=20,
    double learning_rate=0.1){
    check_feature_table(x,y);
    require(rounds>0&&rounds<=1000&&std::isfinite(learning_rate)&&
            learning_rate>0&&learning_rate<=1,"Invalid boosting hyperparameters");
    BoostedStumps model;
    model.baseline=mean(y);model.learning_rate=learning_rate;
    std::vector<double> residual(y.size());
    for(int t=0;t<int(y.size());t++)residual[t]=y[t]-model.baseline;
    for(int r=0;r<rounds;r++){
        auto stump=fit_regression_stump(x,residual);
        model.stumps.push_back(stump);
        for(int t=0;t<int(y.size());t++)
            residual[t]-=learning_rate*stump_predict(stump,x[t]);
    }
    return model;
}
inline double boosted_predict(const BoostedStumps& model,
                               const std::vector<double>& features){
    double value=model.baseline;
    for(const auto& stump:model.stumps)
        value+=model.learning_rate*stump_predict(stump,features);
    return value;
}
} // namespace kline
