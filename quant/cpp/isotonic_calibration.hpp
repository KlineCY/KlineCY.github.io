#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct IsotonicModel{
    std::vector<double> upper_scores,probabilities;
};
inline IsotonicModel fit_isotonic(const std::vector<double>& scores,
                                 const std::vector<int>& labels){
    check_finite(scores);
    require(scores.size()==labels.size(),"Isotonic labels differ");
    std::vector<std::pair<double,int>> rows;
    for(int i=0;i<int(scores.size());i++){
        require(labels[i]==0||labels[i]==1,"Isotonic binary labels");
        rows.push_back({scores[i],labels[i]});
    }
    std::stable_sort(rows.begin(),rows.end());
    struct Block{double sum=0;int count=0;double right=0;};
    std::vector<Block> blocks;
    for(const auto& row:rows){
        if(!blocks.empty()&&blocks.back().right==row.first){
            blocks.back().sum+=row.second;
            blocks.back().count++;
        }else blocks.push_back({double(row.second),1,row.first});
        while(blocks.size()>=2){
            int n=int(blocks.size());
            if(blocks[n-2].sum/blocks[n-2].count<=
               blocks[n-1].sum/blocks[n-1].count)break;
            blocks[n-2].sum+=blocks[n-1].sum;
            blocks[n-2].count+=blocks[n-1].count;
            blocks[n-2].right=blocks[n-1].right;
            blocks.pop_back();
        }
    }
    IsotonicModel m;
    for(const auto& b:blocks){
        m.upper_scores.push_back(b.right);
        m.probabilities.push_back(b.sum/b.count);
    }
    return m;
}
inline double isotonic_predict(const IsotonicModel& model,double score){
    require(!model.upper_scores.empty()&&std::isfinite(score),
            "Isotonic model is empty");
    auto it=std::lower_bound(model.upper_scores.begin(),
                            model.upper_scores.end(),score);
    int i=it==model.upper_scores.end()?int(model.upper_scores.size())-1:
          int(it-model.upper_scores.begin());
    return model.probabilities[i];
}
} // namespace kline
