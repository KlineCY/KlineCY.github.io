#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
inline double classification_auc(const std::vector<double>& probability,
                                  const std::vector<int>& labels) {
    check_finite(probability);
    require(probability.size()==labels.size(),"AUC labels and scores differ");
    auto rank=ranks(probability);
    double positives=0,negatives=0,sum=0;
    for(int i=0;i<int(labels.size());i++){
        require(labels[i]==0||labels[i]==1,"AUC labels must be binary");
        if(labels[i]==1){positives++;sum+=rank[i];}
        else negatives++;
    }
    require(positives>0&&negatives>0,"AUC requires two classes");
    return (sum-positives*(positives+1)/2)/(positives*negatives);
}

inline double binary_log_loss(const std::vector<double>& probability,
                              const std::vector<int>& labels) {
    check_finite(probability);
    require(probability.size()==labels.size(),"Log loss dimensions differ");
    double sum=0;
    for(int i=0;i<int(probability.size());i++){
        require((labels[i]==0||labels[i]==1)&&probability[i]>=0&&
                probability[i]<=1,"Invalid binary probability or label");
        double p=std::max(1e-15,std::min(1-1e-15,probability[i]));
        sum-=labels[i]*std::log(p)+(1-labels[i])*std::log1p(-p);
    }
    return sum/probability.size();
}

struct ClassificationCounts {
    int tp=0,fp=0,tn=0,fn=0;
};

inline ClassificationCounts binary_confusion(
    const std::vector<double>& probability,const std::vector<int>& labels,
    double threshold=0.5) {
    check_finite(probability);
    require(probability.size()==labels.size()&&std::isfinite(threshold)&&
            threshold>=0&&threshold<=1,"Invalid binary decision threshold");
    ClassificationCounts c;
    for(int i=0;i<int(probability.size());i++){
        require((labels[i]==0||labels[i]==1)&&probability[i]>=0&&
                probability[i]<=1,"Invalid prediction or binary label");
        bool guess=probability[i]>=threshold;
        if(guess&&labels[i]==1)c.tp++;
        else if(guess)c.fp++;
        else if(labels[i]==0)c.tn++;
        else c.fn++;
    }
    return c;
}

inline double f1_score(const ClassificationCounts& c) {
    require(c.tp>=0&&c.fp>=0&&c.fn>=0,"Invalid confusion counts");
    int denominator=2*c.tp+c.fp+c.fn;
    return denominator?double(2*c.tp)/denominator:0;
}

inline double out_of_sample_r_squared(const std::vector<double>& predicted,
                                      const std::vector<double>& realized,
                                      double training_mean) {
    check_finite(predicted);check_finite(realized);
    require(predicted.size()==realized.size()&&std::isfinite(training_mean),
            "OOS R squared arrays differ");
    double model=0,benchmark=0;
    for(int i=0;i<int(predicted.size());i++){
        model+=std::pow(realized[i]-predicted[i],2);
        benchmark+=std::pow(realized[i]-training_mean,2);
    }
    require(benchmark>0,"OOS benchmark error is zero");
    return 1-model/benchmark;
}

inline double precision_at_k(const std::vector<double>& scores,
                              const std::vector<int>& labels,int k) {
    check_finite(scores);
    require(scores.size()==labels.size()&&k>0&&k<=int(scores.size()),
            "Invalid precision-at-k dimensions");
    std::vector<int> order(scores.size());
    std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),
                     [&](int a,int b){return scores[a]>scores[b];});
    int positives=0;
    for(int i=0;i<int(labels.size());i++)
        require(labels[i]==0||labels[i]==1,"Invalid ranking label");
    for(int i=0;i<k;i++)positives+=labels[order[i]];
    return double(positives)/k;
}
} // namespace kline
