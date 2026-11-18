#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double binary_auc_mann_whitney(
    const std::vector<double>& scores,const std::vector<int>& labels){
    check_finite(scores);
    require(scores.size()==labels.size(),"AUC score-label dimensions");
    std::vector<std::pair<double,int>> rows;
    int positive=0,negative=0;
    for(int i=0;i<int(scores.size());i++){
        require(labels[i]==0||labels[i]==1,"AUC binary label");
        rows.push_back({scores[i],labels[i]});
        positive+=labels[i];negative+=1-labels[i];
    }
    require(positive>0&&negative>0,"AUC needs both classes");
    std::sort(rows.begin(),rows.end());
    double rank_sum=0;
    int index=0;
    while(index<int(rows.size())){
        int end=index;
        while(end<int(rows.size())&&rows[end].first==rows[index].first)end++;
        double rank=0.5*(index+1+end);
        for(int k=index;k<end;k++)
            if(rows[k].second)rank_sum+=rank;
        index=end;
    }
    return (rank_sum-positive*(positive+1.0)/2)/(positive*double(negative));
}
inline double binary_brier_score(
    const std::vector<double>& probabilities,
    const std::vector<int>& outcomes){
    check_finite(probabilities);
    require(probabilities.size()==outcomes.size(),"Brier dimensions");
    double loss=0;
    for(int i=0;i<int(probabilities.size());i++){
        require(probabilities[i]>=0&&probabilities[i]<=1&&
                (outcomes[i]==0||outcomes[i]==1),
                "Brier probabilities or labels invalid");
        loss+=std::pow(probabilities[i]-outcomes[i],2)/outcomes.size();
    }
    return loss;
}
inline double binary_logarithmic_score(
    const std::vector<double>& probabilities,
    const std::vector<int>& outcomes,double epsilon=1e-12){
    check_finite(probabilities);
    require(probabilities.size()==outcomes.size()&&epsilon>0&&
            epsilon<0.5,"Invalid classification log score");
    double sum=0;
    for(int i=0;i<int(probabilities.size());i++){
        double p=probabilities[i];
        require(p>=0&&p<=1&&(outcomes[i]==0||outcomes[i]==1),
                "Invalid binary probability or label");
        p=std::max(epsilon,std::min(1-epsilon,p));
        sum-=(outcomes[i]?std::log(p):std::log(1-p))/outcomes.size();
    }
    return sum;
}
} // namespace kline
