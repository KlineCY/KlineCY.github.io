#pragma once
#include "stats.hpp"
#include <algorithm>
#include <functional>
#include <random>
#include <vector>
namespace kline {
inline double permutation_importance_mse(
    const Matrix& test_features,const std::vector<double>& labels,
    const std::function<double(const std::vector<double>&)>& predictor,
    int feature,unsigned int seed,int repetitions=10){
    check_feature_table(test_features,labels);
    require(feature>=0&&feature<int(test_features[0].size())&&
            repetitions>0,"Invalid test feature permutation");
    double baseline=0;
    for(int t=0;t<int(labels.size());t++)
        baseline+=std::pow(labels[t]-predictor(test_features[t]),2)/
                  labels.size();
    std::vector<double> shuffled;
    for(const auto& row:test_features)shuffled.push_back(row[feature]);
    std::mt19937 rng(seed);double difference=0;
    for(int r=0;r<repetitions;r++){
        std::shuffle(shuffled.begin(),shuffled.end(),rng);
        double mse=0;
        for(int t=0;t<int(labels.size());t++){
            auto row=test_features[t];row[feature]=shuffled[t];
            mse+=std::pow(labels[t]-predictor(row),2)/labels.size();
        }
        difference+=(mse-baseline)/repetitions;
    }
    return difference;
}
} // namespace kline
