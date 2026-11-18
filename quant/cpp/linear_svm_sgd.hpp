#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct LinearSvm{std::vector<double> weights;double intercept=0;};
inline LinearSvm fit_linear_svm_sgd(
    const Matrix& features,const std::vector<double>& signed_labels,
    double lambda,int epochs=100,double learning_rate=0.1){
    check_feature_table(features,signed_labels);
    require(std::isfinite(lambda)&&lambda>0&&epochs>0&&
            std::isfinite(learning_rate)&&learning_rate>0,
            "SVM parameters require positive regularization and learning rate");
    int p=int(features[0].size());
    for(double y:signed_labels)
        require(y==1||y==-1,"Linear SVM labels must be -1 or +1");
    LinearSvm result;result.weights.assign(p,0);
    int iteration=0;
    for(int epoch=0;epoch<epochs;epoch++){
        for(int i=0;i<int(features.size());i++){
            double y=signed_labels[i],margin=result.intercept;
            for(int j=0;j<p;j++)
                margin+=result.weights[j]*features[i][j];
            double eta=learning_rate/std::sqrt(++iteration);
            bool violation=y*margin<1;
            for(int j=0;j<p;j++){
                double grad=lambda*result.weights[j]-
                             (violation?y*features[i][j]:0.0);
                result.weights[j]-=eta*grad;
            }
            if(violation)result.intercept+=eta*y;
        }
    }
    return result;
}
inline double svm_decision_score(const LinearSvm& model,
                                 const std::vector<double>& row){
    check_finite(row);
    require(row.size()==model.weights.size(),"SVM features do not match");
    double score=model.intercept;
    for(int j=0;j<int(row.size());j++)score+=model.weights[j]*row[j];
    return score;
}
} // namespace kline
