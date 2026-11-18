#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<int> nearest_neighbors(const Matrix& training,
    const std::vector<double>& query,int k){
    require(!training.empty()&&k>0&&k<=int(training.size()),
            "KNN sample size");
    check_finite(query);
    std::vector<std::pair<double,int>> neighbors;
    for(int i=0;i<int(training.size());i++){
        check_finite(training[i]);
        require(training[i].size()==query.size(),"KNN feature dimension");
        double d=0;
        for(int j=0;j<int(query.size());j++)
            d+=std::pow(training[i][j]-query[j],2);
        neighbors.push_back({d,i});
    }
    std::stable_sort(neighbors.begin(),neighbors.end());
    std::vector<int> result;
    for(int i=0;i<k;i++)result.push_back(neighbors[i].second);
    return result;
}
inline double knn_regression(const Matrix& x,const std::vector<double>& y,
                             const std::vector<double>& query,int k){
    check_feature_table(x,y);
    auto ids=nearest_neighbors(x,query,k);
    double prediction=0;
    for(int i:ids)prediction+=y[i]/k;
    return prediction;
}
inline double knn_binary_probability(const Matrix& x,
    const std::vector<int>& labels,const std::vector<double>& query,int k){
    require(x.size()==labels.size(),"KNN labels mismatch");
    for(int y:labels)require(y==0||y==1,"KNN requires binary labels");
    auto ids=nearest_neighbors(x,query,k);
    double score=0;
    for(int id:ids)score+=double(labels[id])/k;
    return score;
}
} // namespace kline
