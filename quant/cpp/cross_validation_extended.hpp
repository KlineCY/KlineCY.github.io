#pragma once
#include "validation_advanced.hpp"
#include <algorithm>
#include <numeric>
#include <vector>
namespace kline {
struct PurgedFold{
    std::vector<int> training,test;
};
inline std::vector<PurgedFold> purged_kfold(
    const std::vector<LabeledPeriod>& events,int folds,int embargo=0){
    require(events.size()>=4&&folds>=2&&folds<=int(events.size())&&
            embargo>=0,"Invalid purged K-fold parameters");
    for(int i=0;i<int(events.size());i++){
        require(events[i].end>events[i].start,"Invalid event period");
        if(i>0)require(events[i].start>=events[i-1].start,
                       "Events must be chronologically ordered");
    }
    std::vector<PurgedFold> out;
    int n=int(events.size());
    for(int fold=0;fold<folds;fold++){
        int begin=fold*n/folds,end=(fold+1)*n/folds;
        int test_start=events[begin].start,test_end=events[end-1].end;
        for(int i=begin;i<end;i++)test_end=std::max(test_end,events[i].end);
        PurgedFold f;
        for(int i=begin;i<end;i++)f.test.push_back(i);
        for(int i=0;i<n;i++){
            if(i>=begin&&i<end)continue;
            bool overlap=events[i].start<test_end&&events[i].end>test_start;
            bool embargoed=events[i].start>=test_end&&
                            events[i].start<test_end+embargo;
            if(!overlap&&!embargoed)f.training.push_back(i);
        }
        out.push_back(f);
    }
    return out;
}
inline std::vector<int> expanding_train_endpoints(int observations,
    int first_train,int step,int horizon){
    require(first_train>=2&&step>=1&&horizon>=1&&
            observations>first_train,"Invalid rolling endpoints");
    std::vector<int> endpoints;
    for(int end=first_train;end+horizon<=observations;end+=step)
        endpoints.push_back(end);
    return endpoints;
}
} // namespace kline
