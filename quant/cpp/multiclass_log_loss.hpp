#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
inline double multiclass_negative_log_likelihood(
    const Matrix& probabilities,const std::vector<int>& labels,
    double probability_floor=1e-12){
    require(!probabilities.empty()&&probabilities.size()==labels.size()&&
            std::isfinite(probability_floor)&&probability_floor>0&&
            probability_floor<1,"Invalid multiclass log loss shape");
    int classes=int(probabilities.front().size());
    require(classes>=2,"Multiclass log loss needs at least two classes");
    double loss=0;
    for(int i=0;i<int(labels.size());i++){
        check_finite(probabilities[i]);
        require(int(probabilities[i].size())==classes&&
                labels[i]>=0&&labels[i]<classes,
                "Multiclass prediction dimension or label");
        double total=0;
        for(double p:probabilities[i]){
            require(p>=0&&p<=1,"Invalid multiclass probability");
            total+=p;
        }
        require(std::fabs(total-1)<1e-8,"Class probabilities must sum to one");
        loss-=std::log(std::max(probability_floor,
                               probabilities[i][labels[i]]))/labels.size();
    }
    return loss;
}
} // namespace kline
