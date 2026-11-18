#pragma once
#include "tree_models.hpp"
#include <random>
#include <vector>
namespace kline {
struct BaggedStumps{
    std::vector<RegressionStump> trees;
    int predictors=0;
};
inline BaggedStumps fit_bagged_stumps(const Matrix& x,
                const std::vector<double>& y,int trees,unsigned int seed){
    check_feature_table(x,y);
    require(trees>=1&&trees<=10000&&x.size()>=3,
            "Invalid bagged stumps sample size");
    std::mt19937 engine(seed);
    std::uniform_int_distribution<int> draw(0,int(x.size())-1);
    BaggedStumps m;m.predictors=int(x[0].size());
    for(int t=0;t<trees;t++){
        Matrix boot;std::vector<double> target;
        for(int i=0;i<int(x.size());i++){
            int id=draw(engine);
            boot.push_back(x[id]);target.push_back(y[id]);
        }
        try {m.trees.push_back(fit_regression_stump(boot,target));}
        catch(const std::invalid_argument&){
            // A bootstrap sample can be constant; use the original sample.
            m.trees.push_back(fit_regression_stump(x,y));
        }
    }
    return m;
}
inline double bagged_stumps_prediction(const BaggedStumps& model,
                                      const std::vector<double>& features){
    check_finite(features);
    require(!model.trees.empty()&&int(features.size())==model.predictors,
            "Invalid bagged stumps prediction");
    double total=0;
    for(const auto& tree:model.trees)
        total+=stump_predict(tree,features)/model.trees.size();
    return total;
}
} // namespace kline
