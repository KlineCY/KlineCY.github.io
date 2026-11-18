#pragma once
#include "validation.hpp"
#include <algorithm>
#include <vector>

namespace kline {
struct LabeledPeriod {
    int start=0;
    int end=0;
};

inline std::vector<int> purge_label_intervals(
    const std::vector<LabeledPeriod>& training,
    const LabeledPeriod& test,int embargo=0) {
    require(test.end>test.start&&embargo>=0,
            "Invalid validation interval or embargo");
    std::vector<int> keep;
    for(int i=0;i<int(training.size());i++){
        auto event=training[i];
        require(event.end>event.start,"Invalid training event interval");
        bool overlaps=event.start<test.end&&event.end>test.start;
        bool embargoed=event.start>=test.end&&
                        event.start<test.end+embargo;
        if(!overlaps&&!embargoed)keep.push_back(i);
    }
    return keep;
}

inline int eligible_training_count(const std::vector<int>& maturity,
                                    int decision,int gap=0) {
    require(!maturity.empty()&&decision>=0&&gap>=0&&decision>=gap,
            "Invalid label maturity gate");
    int cutoff=decision-gap,count=0;
    for(int i=0;i<int(maturity.size());i++){
        require(maturity[i]>i,"Label maturity must follow feature time");
        if(i<cutoff&&maturity[i]<=cutoff)count++;
    }
    return count;
}

inline std::vector<int> chronological_holdout_indices(
    int observations,int last_test_count,int gap=0) {
    require(observations>=3&&last_test_count>0&&gap>=0&&
            last_test_count+gap<observations,
            "Invalid chronological holdout parameters");
    std::vector<int> labels;
    for(int t=observations-last_test_count;t<observations;t++)
        labels.push_back(t);
    return labels;
}
} // namespace kline
