#pragma once
#include "stats.hpp"
#include <algorithm>
#include <vector>
namespace kline {
struct IsotonicReal{
    std::vector<double> score_end,mean;
};
inline IsotonicReal fit_isotonic_real(
    const std::vector<double>& scores,const std::vector<double>& outcomes){
    check_finite(scores);check_finite(outcomes);
    require(scores.size()==outcomes.size(),"Isotonic real sample mismatch");
    std::vector<std::pair<double,double>> pairs;
    for(int i=0;i<int(scores.size());i++)
        pairs.push_back({scores[i],outcomes[i]});
    std::stable_sort(pairs.begin(),pairs.end());
    struct Block{double total=0;int observations=0;double right=0;};
    std::vector<Block> blocks;
    for(const auto& item:pairs){
        if(!blocks.empty()&&blocks.back().right==item.first){
            blocks.back().total+=item.second;blocks.back().observations++;
        }else blocks.push_back({item.second,1,item.first});
        while(blocks.size()>1){
            int k=int(blocks.size());
            if(blocks[k-2].total/blocks[k-2].observations<=
               blocks[k-1].total/blocks[k-1].observations)break;
            blocks[k-2].total+=blocks[k-1].total;
            blocks[k-2].observations+=blocks[k-1].observations;
            blocks[k-2].right=blocks[k-1].right;
            blocks.pop_back();
        }
    }
    IsotonicReal m;
    for(auto b:blocks){
        m.score_end.push_back(b.right);
        m.mean.push_back(b.total/b.observations);
    }
    return m;
}
inline double isotonic_real_predict(const IsotonicReal& m,double query){
    require(!m.score_end.empty()&&std::isfinite(query),
            "Invalid real isotonic prediction");
    auto it=std::lower_bound(m.score_end.begin(),m.score_end.end(),query);
    int k=it==m.score_end.end()?int(m.mean.size())-1:
          int(it-m.score_end.begin());
    return m.mean[k];
}
} // namespace kline
